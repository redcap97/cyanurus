/*
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
*/

/*
$fixture copy_test_target
$check echo
$disable_echo_on_check
*/
TEST(unistd_fork);

/*
$fixture copy_sbin_unistd_execve
*/
TEST(unistd_execve);

/*
$fixture copy_test_target
*/
TEST(unistd_execve_failed);

/*
$fixture copy_test_target
*/
TEST(unistd_wait);

/*
$fixture copy_test_target
*/
TEST(unistd_exit);

/*
$fixture copy_test_target
$fixture copy_usr_share
*/
TEST(unistd_read);

/*
$fixture copy_test_target
$fixture mkdir_tmp
$check unistd_write
*/
TEST(unistd_write);

/*
$fixture copy_test_target
$fixture write_until_disk_full
*/
TEST(unistd_write_ENOSPC);

/*
$fixture copy_test_target
$check unistd_mkdir
*/
TEST(unistd_mkdir);

/*
$fixture copy_test_target
$fixture mkdir_tmp
$check unistd_rmdir
*/
TEST(unistd_rmdir);

/*
$fixture copy_test_target
*/
TEST(unistd_ioctl_TIOCGWINSZ);

/*
$fixture copy_sbin_unistd_fcntl_F_SETFD
*/
TEST(unistd_fcntl_F_SETFD);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_fcntl_F_SETFL);

/*
$fixture copy_test_target
$check echo
$disable_echo_on_check
*/
TEST(unistd_pipe);

/*
$fixture copy_test_target
*/
TEST(unistd_pipe_SIGPIPE);

/*
$fixture copy_test_target
*/
TEST(unistd_pipe_EPIPE);

/*
$fixture copy_test_target
*/
TEST(unistd_pipe2);

/*
$fixture copy_test_target
*/
TEST(unistd_open);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_open_O_APPEND);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_open_O_TRUNC);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_open_O_CREAT);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_open_O_EXCL);

/*
$fixture copy_test_target
$fixture mkdir_tmp
$fixture write_until_disk_full
*/
TEST(unistd_open_ENOSPC);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_unlink);

/*
$fixture copy_test_target
*/
TEST(unistd_getcwd);

/*
$fixture copy_test_target
*/
TEST(unistd_syscall_EFAULT);

/*
$fixture copy_test_target
$fixture mkdir_tmp
*/
TEST(unistd_lseek);
