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

require 'coop/resource'

class Coop::Session
  TIMEOUT = 4.0

  Response = Struct.new(:result, :output)

  attr_reader :resource

  def self.respawn(kernel, session)
    session.close

    session = new(kernel)
    session.run

    session
  end

  def initialize(kernel)
    unless FileTest.exists?(kernel)
      raise ArgumentError, "kernel doesn't exist: #{kernel}"
    end

    @resource = Coop::Resource.create
    @cyanurus = Coop::Cyanurus.new(@resource)
    @qemu = Coop::Qemu.new(kernel, @resource)

    @threads = []
    @ready = false
  end

  def run
    @threads << @cyanurus.run
    @threads << @qemu.run

    check_is_ready
  end

  def exec_run(name, stdin_data:)
    @cyanurus.send("$run #{name}")

    if stdin_data
      @cyanurus.add_input(stdin_data)
    end

    t = Thread.new do
      output = @cyanurus.recv
      result = @cyanurus.recv

      Response.new(result, output)
    end

    if t.join(TIMEOUT)
      check_is_ready
      t.value
    else
      t.kill
      @ready = false

      result = Coop::Message.new('failure', '(unresponsive)')
      output = Coop::Message.new('echo', @cyanurus.unprocessed_data)

      Response.new(result, output)
    end
  end

  def close
    @qemu.add_command("quit")

    @threads.each do |t|
      t.join
    end

    @resource.release
  end

  def ready?
    @ready
  end

  private

  def check_is_ready
    message = @cyanurus.recv
    @ready = (message.name == "please")
  end
end
