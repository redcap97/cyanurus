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

require 'coop/cyanurus'
require 'coop/qemu'
require 'coop/session'
require 'coop/entry_parser'
require 'coop/statistics'
require 'coop/message/fixture'
require 'coop/message/check'
require 'coop/test'

require 'erb'
require 'optparse'

class Coop::Application
  KERNEL = 'cyanurus.elf'

  VALID_MESSAGES = %w(
    stdin
    fixture
    check
    disable_echo_on_check
    shutdown
  )

  attr_reader :src_path
  attr_reader :stats

  def initialize(src_path)
    @src_path = src_path
    @stats = Coop::Statistics.new(STDOUT)
  end

  def run
    validate_entries!

    session = Coop::Session.new(KERNEL)
    session.run

    tests = entries.keys
    tests.each do |name|
      messages = entries[name] || []

      test = Coop::Test.new(name, messages, self, session)
      test.run

      if !session.ready? || test.shutdown_required?
        session = Coop::Session.respawn(KERNEL, session)
      end
    end

    @stats.report
  ensure
    session.close unless session.nil?
  end

  def list_entry_files
    get_entry_files.each do |file|
      STDOUT.puts file
    end
  end

  def dump_entries
    templ = <<-EOF
<% entry_files.each do |file| %>
#include "<%= file %>"
<% end %>

struct test_entry test_entries[] = {
<% entries.keys.each do |name| %>
  TEST_ENTRY(<%= name %>),
<% end %>
  TEST_ENTRY_NULL,
};
    EOF

    erb = ERB.new(templ, nil, '<>')
    STDOUT.puts erb.result(binding())
  end

  def success?
    @stats.success?
  end

  private

  def entry_files
    @entry_files ||= get_entry_files
  end

  def entries
    @entries ||= get_entries
  end

  def validate_entries!
    entries.each do |name, messages|
      remaining = messages.map(&:name) - VALID_MESSAGES

      unless remaining.empty?
        error_message = "TEST(#{name}) has invalid messages: #{remaining.inspect}"
        raise Coop::ValidationError, error_message
      end
    end
  end

  def get_entry_files
    paths = []

    pattern = File.join(@src_path, '**/*.t')
    Dir.glob(pattern) do |path|
      paths << path
    end

    paths
  end

  def get_entries
    entries = {}

    entry_files.each do |path|
      open(path) do |f|
        entries.merge!(Coop::EntryParser.parse(f.read))
      end
    end

    entries
  end
end
