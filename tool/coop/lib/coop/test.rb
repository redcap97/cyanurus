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

class Coop::Test
  def initialize(name, messages, app, session)
    @name     = name
    @messages = messages
    @app      = app
    @session  = session
  end

  def run
    prepare_fixture
    response = @session.exec_run(@name, stdin_data: stdin_data)
    handle_response(response)
  end

  def shutdown_required?
    @messages.any?{|m| m.name == 'shutdown'}
  end

  private

  def stats
    @app.stats
  end

  def disabled_echo_on_check?
    @messages.any?{|m| m.name == 'disable_echo_on_check'}
  end

  def prepare_fixture
    fixture = Coop::Message::Fixture.new(@app, @session)

    @messages.each do |message|
      fixture.dispatch(@name, message.body) if message.name == 'fixture'
    end
  end

  def stdin_data
    @messages.each do |message|
      return message.body if message.name == 'stdin'
    end

    nil
  end

  def check_evidence(response)
    check = Coop::Message::Check.new(@app, @session)
    messages = @messages.select{|message| message.name == 'check' }

    if messages.empty?
      stats.fail(@name, '($check not found)')
      return
    end

    messages.each do |message|
      check.dispatch(@name, message.body, response)
    end
  end

  def handle_response(response)
    result = response.result
    output = response.output

    if output.name != 'echo'
      STDERR.puts "unknown message: #{output.name}"
      return
    end

    case result.name
    when 'success'
      stats.succeed(@name)
    when 'failure'
      stats.fail(@name, result.body)
    when 'check'
      check_evidence(response)
      return if disabled_echo_on_check?
    else
      STDERR.puts "unknown message: #{result.name}"
    end

    print(output.body)
  end
end
