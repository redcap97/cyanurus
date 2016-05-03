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

require 'open3'

class Coop::Message::Check
  def initialize(app, session)
    @root_path = app.root_path
    @source_path = app.source_path
    @resource = session.resource
    @stats    = app.stats
  end

  def dispatch(test, name, response)
    unless /\A[A-Za-z0-9_]+\z/ =~ name
      raise ArgumentError, "Invalid name: #{name}"
    end

    output = response.output

    env = {
      'TEST_NAME'   => test,
      'BUILD_PATH'  => Dir.getwd,
      'ROOT_PATH'   => @root_path,
      'SOURCE_PATH' => @source_path,
    }

    command = File.join(@source_path, 'check', name)

    Dir.chdir(@resource.dir) do
      stdin_data = output.body.gsub(/\r?\n/, "\n")
      out, err, status = Open3.capture3(env, command, stdin_data: stdin_data)

      if status.success?
        @stats.succeed(test)
      else
        @stats.fail(test, '(on check)')
      end

      print(out)
      print(err)
    end
  end
end
