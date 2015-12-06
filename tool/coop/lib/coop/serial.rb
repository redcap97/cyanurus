#coding: utf-8

=begin
Copyright 2014 Akira Midorikawa

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
=end

require 'socket'

class Coop::Serial
  def initialize(endpoint)
    @thread = nil

    @input = Queue.new
    @output = Queue.new
    @endpoint = endpoint
  end

  def run
    @thread = Thread.new do
      Thread.handle_interrupt(Coop::AbortSelect => :never) { dispatch }
    end
  end

  def add_input(str)
    @input << str
    @thread.raise Coop::AbortSelect
  end

  def pop_output
    @output.pop
  end

  private

  def add_output(str)
    @output << str
  end

  def pop_input
    @input.pop(true) rescue nil
  end

  def input_empty?
    @input.empty?
  end

  def dispatch
    Socket.unix_server_loop(@endpoint) do |sock, addr|
      start_loop(sock)
      break
    end
  rescue EOFError, Errno::EIO, Errno::ECONNRESET
  end

  def start_loop(sock)
    rs = nil
    ws = nil

    loop do
      Thread.handle_interrupt(Coop::AbortSelect => :immediate) do
        begin
          rs, ws, = IO.select([sock], (input_empty? ? [] : [sock]))
        rescue Coop::AbortSelect
          retry
        end
      end

      unless rs.empty?
        add_output(sock.read_nonblock(4096))
      end

      unless ws.empty?
        if input = pop_input
          sock.write_nonblock(input)
        end
      end

      Thread.pass
    end
  end
end
