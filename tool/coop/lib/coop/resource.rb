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

require 'fileutils'
require 'tmpdir'

class Coop::Resource
  attr_reader :dir

  def self.create
    new.tap(&:prepare)
  end

  def initialize
    @dir = Dir.mktmpdir('coop')
  end

  def sock
    File.join(@dir, 'sock')
  end

  def disk
    File.join(@dir, 'disk.img')
  end

  def prepare
    options = {out: '/dev/null'}

    system('qemu-img', 'create', '-f', 'raw', disk, '64M', options)
    system('mkfs.mfs', '-B', '4096', disk, options)
  end

  def release
    if Dir.exists?(@dir)
      FileUtils.remove_entry_secure(@dir)
    end
  end
end
