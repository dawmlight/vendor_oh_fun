/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import Core from './src/core'
import InstrumentLog from './src/module/report/InstrumentLog'
import ReportExtend from './src/module/report/ReportExtend'
import ExpectExtend from './src/module/assert/ExpectExtend'
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from './src/interface'

export {
  Core,
  InstrumentLog,
  ReportExtend,
  ExpectExtend,
  describe, beforeAll, beforeEach, afterEach, afterAll, it, expect
}
