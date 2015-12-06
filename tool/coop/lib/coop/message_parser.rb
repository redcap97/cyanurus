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

class Coop::MessageParser
  def self.parse(str, eol: "\n")
    i = str.index(eol)
    return unless i

    line = str.slice(0, i)

    case line[0]
    when "$"
      str.slice!(0, i + eol.size)
      name, arg = line.split(' ', 2)

      Coop::Message.new(name[1..-1], arg)
    when ":"
      name, sig = line.split(' ')
      last = eol + sig + eol

      n = str.index(last)
      return unless n

      lines = str.slice((i + eol.size), n - (i + eol.size))
      str.slice!(0, n + last.size)

      Coop::Message.new(name[1..-1], lines)
    else
      raise Coop::InvalidFormat, line.inspect
    end
  end
end
