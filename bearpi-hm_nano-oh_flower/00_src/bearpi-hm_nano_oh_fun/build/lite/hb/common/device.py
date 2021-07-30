#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import os
import re

from hb.common.config import Config
from hb.cts.menuconfig import Menuconfig


class Device():
    @staticmethod
    def is_in_device():
        cwd_pardir = os.path.dirname(os.path.dirname(os.getcwd()))
        config = Config()
        return os.path.join(config.root_path, 'device') == cwd_pardir

    @staticmethod
    def is_kernel(kernel_path):
        return os.path.isdir(kernel_path) and\
            'config.gni' in os.listdir(kernel_path)

    @staticmethod
    def device_menuconfig():
        kernel_path_dict = {}
        cwd = os.getcwd()
        Device.check_path(cwd)

        for kernel_config, kernel_path in Device.get_kernel_config(cwd):
            kernel_type, kernel_version = Device.get_kernel_info(kernel_config)
            kernel_path_dict['{}@{}'.format(kernel_type, kernel_version)] =\
                kernel_path

        if not len(kernel_path_dict):
            raise Exception('no valid kernel found')

        choices = [{'name': kernel} for kernel in kernel_path_dict.keys()]

        menu = Menuconfig()
        kernel = menu.list_promt('kernel',
                                 'Which kernel do you need?',
                                 choices).get('kernel')
        return kernel_path_dict.get(kernel), kernel.split('@')[0],\
            os.path.basename(cwd)

    @staticmethod
    def get_device_path(board_path, kernel_type, kernel_version):
        for kernel_config, kernel_path in Device.get_kernel_config(board_path):
            if Device.match_kernel(kernel_config,
                                   kernel_type,
                                   kernel_version):
                return kernel_path

        raise Exception('cannot find {}_{} in {}'.format(kernel_type,
                                                         kernel_version,
                                                         board_path))

    @staticmethod
    def get_kernel_config(board_path):
        Device.check_path(board_path)
        for kernel in os.listdir(board_path):
            kernel_path = os.path.join(board_path, kernel)

            if os.path.isdir(kernel_path):
                kernel_config = os.path.join(kernel_path, 'config.gni')
                if not os.path.isfile(kernel_config):
                    continue
                yield kernel_config, kernel_path

    @staticmethod
    def match_kernel(config, kernel, version):
        kernel_pattern = r'kernel_type ?= ?"{}"'.format(kernel)
        version_pattern = r'kernel_version ?= ?"{}"'.format(version)

        with open(config, 'rt', encoding='utf-8') as config_file:
            data = config_file.read()
            return re.search(kernel_pattern, data) and\
                re.search(version_pattern, data)

    @staticmethod
    def get_kernel_info(config):
        kernel_pattern = r'kernel_type ?= ?"(\w+)"'
        version_pattern = r'kernel_version ?= ?"([a-zA-Z0-9._]*)"'

        with open(config, 'rt', encoding='utf-8') as config_file:
            data = config_file.read()
            kernel_list = re.findall(kernel_pattern, data)
            version_list = re.findall(version_pattern, data)
            if not len(kernel_list) or not len(version_list):
                raise Exception('kernel_type or kernel_version '
                                'not found in {}'.format(config))

            return kernel_list[0], version_list[0]

    @staticmethod
    def check_path(path):
        if os.path.isdir(path) or os.path.isfile(path):
            return
        raise Exception('invalid path: {}'.format(path))
