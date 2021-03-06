#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
#    to endorse or promote products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import json

from .hdf_command_handler_base import HdfCommandHandlerBase
from .hdf_lite_mk_file import HdfLiteMkFile
from .hdf_vendor_kconfig_file import HdfVendorKconfigFile
from .hdf_module_kconfig_file import HdfModuleKconfigFile
from .hdf_driver_config_file import HdfDriverConfigFile
from command_line.hdf_command_error_code import CommandErrorCode
from hdf_tool_settings import HdfToolSettings
from hdf_tool_exception import HdfToolException
import hdf_tool_version
import hdf_utils


class HdfGetHandler(HdfCommandHandlerBase):
    def __init__(self, args):
        super(HdfGetHandler, self).__init__()
        self.cmd = 'get'
        self.handlers = {
            'vendor_list': self._get_vendor_list_handler,
            'current_vendor': self._get_current_vendor_handler,
            'vendor_parent_path': self._get_vendor_parent_path_handler,
            'individual_vendor_path': self._get_individual_vendor_path_handler,
            'board_list': self._get_board_list_handler,
            'driver_list': self._get_driver_list_handler,
            'driver_file': self._get_driver_file_handler,
            'drv_config_file': self._get_drv_config_file_handler,
            'hdf_tool_core_version': self._get_version_handler
        }
        self.parser.add_argument("--action_type",
                                 help=' '.join(self.handlers.keys()),
                                 required=True)
        self.parser.add_argument("--root_dir")
        self.parser.add_argument("--vendor_name")
        self.parser.add_argument("--module_name")
        self.parser.add_argument("--driver_name")
        self.parser.add_argument("--board_name")
        self.args = self.parser.parse_args(args)

    def _get_vendor_list_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        root = self.args.root_dir
        vendor_root_dir = hdf_utils.get_vendor_root_dir(root)
        vendors = []
        if os.path.exists(vendor_root_dir):
            for vendor in os.listdir(vendor_root_dir):
                hdf = hdf_utils.get_vendor_hdf_dir(root, vendor)
                if os.path.exists(hdf):
                    vendors.append(vendor)
        return ','.join(vendors)

    def _get_current_vendor_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        return HdfLiteMkFile(self.args.root_dir).get_current_vendor()

    @staticmethod
    def _get_board_list_handler():
        settings = HdfToolSettings()
        return settings.get_supported_boards()

    def _get_vendor_parent_path_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        target = hdf_utils.get_vendor_root_dir(self.args.root_dir)
        return os.path.realpath(target)

    def _get_individual_vendor_path_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        self.check_arg_raise_if_not_exist("vendor_name")
        root, vendor, _, _, _ = self.get_args()
        target = hdf_utils.get_vendor_dir(root, vendor)
        return os.path.realpath(target)

    @staticmethod
    def _get_version_handler():
        return hdf_tool_version.get_version()

    def _get_driver_list_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        self.check_arg_raise_if_not_exist("vendor_name")
        root, vendor, _, _, _ = self.get_args()
        hdf_dir = hdf_utils.get_vendor_hdf_dir(root, vendor)
        if not os.path.exists(hdf_dir):
            raise HdfToolException('vendor "%s" not exist' % vendor,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        modules = os.listdir(hdf_dir)
        vendor_k = HdfVendorKconfigFile(root, vendor)
        module_items = vendor_k.get_module_and_config_path()
        drivers = {}
        for item in module_items:
            module, k_path = item
            if module in modules:
                models = \
                    HdfModuleKconfigFile(root, module, k_path).get_models()
                drivers[module] = models
        return json.dumps(drivers)

    def _get_driver_file_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        self.check_arg_raise_if_not_exist("vendor_name")
        self.check_arg_raise_if_not_exist("module_name")
        self.check_arg_raise_if_not_exist("driver_name")
        root = os.path.realpath(self.args.root_dir)
        _, vendor, module, driver, _ = self.get_args()
        drv_dir = hdf_utils.get_drv_dir(root, vendor, module, driver)
        if not os.path.exists(drv_dir):
            raise HdfToolException('driver directory: %s not exist' % drv_dir,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        for root_path, dirs, files in os.walk(drv_dir):
            for file in files:
                if file.endswith('.c'):
                    return os.path.realpath(os.path.join(root_path, file))
        return ''

    def _get_drv_config_file_handler(self):
        self.check_arg_raise_if_not_exist("root_dir")
        self.check_arg_raise_if_not_exist("module_name")
        self.check_arg_raise_if_not_exist("driver_name")
        self.check_arg_raise_if_not_exist("board_name")
        root, _, module, driver, board = self.get_args()
        drv_config = HdfDriverConfigFile(root, board, module, driver, True)
        return drv_config.get_drv_config_path()
