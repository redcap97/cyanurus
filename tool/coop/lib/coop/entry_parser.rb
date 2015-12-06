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

require 'coop/message_parser'

class Coop::EntryParser
  STATE_FUNCTION = :function
  STATE_COMMENT  = :comment

  def self.parse(data)
    new(data).parse
  end

  def initialize(data)
    @data = data
    @state = STATE_FUNCTION

    @comment = ''
    @messages = []

    @entries = {}
  end

  def parse
    @data.each_line do |line|
      line.chomp!

      case @state
      when STATE_FUNCTION
        on_function(line)
      when STATE_COMMENT
        on_comment(line)
      end
    end

    @entries
  end

  private

  def on_function(line)
    case line
    when /^TEST\(([a-zA-Z0-9_]+)\);$/
      @entries[$1] = @messages.dup
      @messages.clear
    when '/*'
      @state = STATE_COMMENT
    when ''
    else
      raise Coop::InvalidFormat, line.inspect
    end
  end

  def on_comment(line)
    case line
    when '*/'
      if /^copyright/i =~ @comment
        @comment = ""
      end

      while message = Coop::MessageParser.parse(@comment)
        @messages << message
      end

      raise Coop::InvalidFormat, @comment.inspect if @comment != ""
      @state = STATE_FUNCTION
    else
      @comment += (line + "\n")
    end
  end
end
