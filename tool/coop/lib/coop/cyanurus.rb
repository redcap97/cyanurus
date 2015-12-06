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

require 'coop/serial'
require 'coop/message_parser'

class Coop::Cyanurus
  def initialize(resource)
    @buffer = ""
    @serial = Coop::Serial.new(resource.sock)
  end

  def run
    @serial.run
  end

  def recv
    while true
      if response = parse(@buffer)
        return response
      end

      @buffer += @serial.pop_output
    end
  end

  def send(message)
    @serial.add_input(message + "\n")
  end

  def add_input(str)
    @serial.add_input(str)
  end

  def unprocessed_data
    @buffer
  end

  private

  def parse(str)
    Coop::MessageParser.parse(str, eol: "\r\n")
  end
end
