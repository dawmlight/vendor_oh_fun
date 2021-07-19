#!/usr/bin/env python3
# coding=utf-8

#
# Copyright (c) 2020 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import sys
import os
import time
from core.config.config_manager import UserConfigManager


def get_filename_extension(file):
    _, fullname = os.path.split(file)
    filename, ext = os.path.splitext(fullname)
    return filename, ext


def create_dir(path):
    full_path = os.path.abspath(os.path.expanduser(path))
    if not os.path.exists(full_path):
        os.makedirs(full_path, exist_ok=True)


def get_file_list(find_path, postfix=""):
    file_names = os.listdir(find_path)
    file_list = []
    if len(file_names) > 0:
        for file_name in file_names:
            if postfix != "":
                if file_name.find(postfix) != -1 \
                        and file_name[-len(postfix):] == postfix:
                    file_list.append(file_name)
            else:
                file_list.append(file_name)
    return file_list


def get_file_list_by_postfix(path, postfix=""):
    file_list = []
    for dirs in os.walk(path):
        files = get_file_list(find_path=dirs[0], postfix=postfix)
        for file_name in files:
            if "" != file_name and -1 == file_name.find(__file__):
                file_name = os.path.join(dirs[0], file_name)
                if os.path.isfile(file_name):
                    file_list.append(file_name)
    return file_list


def get_device_log_file(report_path, serial=None, log_name="device_log"):
    log_path = os.path.join(report_path, "log")
    os.makedirs(log_path, exist_ok=True)

    serial = serial or time.time_ns()
    device_file_name = "{}_{}.log".format(log_name, serial)
    device_log_file = os.path.join(log_path, device_file_name)
    return device_log_file


def get_build_output_path():
    if sys.source_code_root_path == "":
        return ""

    manager = UserConfigManager()
    if manager.get_user_config_flag("common", "doublefwk"):
        para_dic = manager.get_user_config("build", "paramter")
        target_os = para_dic.get("target_os", "")
        target_cpu = para_dic.get("target_cpu", "")
        variant = para_dic.get("variant", "")
        build_output_name = "%s-%s-%s" % (target_os, target_cpu, variant)
        if build_output_name == "ohos-arm64-release":
            build_output_name = "release"
    else:
        para_dic = manager.get_user_config("build", "board_info")
        board_series = para_dic.get("board_series", "")
        board_type = para_dic.get("board_type", "")
        board_product = para_dic.get("board_product", "")
        first_build_output = "%s_%s" % (board_series, board_type)
        second_build_output = "%s_%s" % (board_product, first_build_output)
        build_output_name = os.path.join(first_build_output,
                                         second_build_output)

    build_output_path = os.path.join(
        sys.source_code_root_path,
        "out",
        build_output_name)
    return build_output_path


def is_32_bit_test():
    manager = UserConfigManager()
    para_dic = manager.get_user_config("build", "paramter")
    target_cpu = para_dic.get("target_cpu", "")
    if target_cpu == "arm":
        return True
    return False


def get_decode(stream):
    if not isinstance(stream, str) and not isinstance(stream, bytes):
        ret = str(stream)
    else:
        try:
            ret = stream.decode("utf-8", errors="ignore")
        except (ValueError, AttributeError, TypeError):
            ret = str(stream)
    return ret
