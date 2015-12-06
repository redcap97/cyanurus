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

require 'pty'

class Coop::Qemu
  COMMAND = %w(
    qemu-system-arm
      -M vexpress-a9
      -m 1G
      -nographic
      -kernel %{kernel}
      -serial %{serial}
      -drive if=sd,file=%{disk},format=raw
      2>/dev/null
  ).join(' ')

  def initialize(kernel, resource)
    @thread = nil
    @commands = Queue.new
    @kernel   = kernel
    @resource = resource
  end

  def run
    @thread = Thread.new do
      Thread.handle_interrupt(Coop::AbortSelect => :never) { dispatch }
    end
  end

  def add_command(command)
    @commands << command
    @thread.raise Coop::AbortSelect
  end

  private

  def pop_command
    @commands.pop(true) rescue nil
  end

  def command_empty?
    @commands.empty?
  end

  def dispatch
    sock = @resource.sock
    disk = @resource.disk

    Thread.pass until FileTest.socket?(sock)
    command = (COMMAND % {kernel: @kernel, serial: 'unix:' + sock, disk: disk})

    PTY.getpty(command) do |read, write, pid|
      start_loop(read, write)
    end
  rescue EOFError, Errno::EIO
  end

  def start_loop(read, write)
    rs = nil
    ws = nil

    loop do
      Thread.handle_interrupt(Coop::AbortSelect => :immediate) do
        begin
          rs, ws, = IO.select([read], (command_empty? ? [] : [write]))
        rescue Coop::AbortSelect
          retry
        end
      end

      unless rs.empty?
        read.read_nonblock(4096)
      end

      unless ws.empty?
        if command = pop_command
          write.write_nonblock(command + "\n")
        end
      end

      Thread.pass
    end
  end
end
