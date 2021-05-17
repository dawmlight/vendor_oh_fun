/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "ability_service.h"

#include "ability_errors.h"
#include "ability_list.h"
#include "ability_message_id.h"
#include "ability_mgr_service.h"
#include "ability_mgr_slite_feature.h"
#include "ability_record.h"
#include "ability_stack.h"
#include "ability_state.h"
#include "abilityms_log.h"
#include "bundle_manager.h"
#include "cmsis_os.h"
#include "js_app_host.h"
#include "los_task.h"
#include "slite_ability.h"
#include "utils.h"
#include "want.h"

using namespace OHOS::ACELite;

namespace OHOS {
constexpr char LAUNCHER_BUNDLE_NAME[] = "com.huawei.launcher";
constexpr uint16_t LAUNCHER_TOKEN = 0;
constexpr int32_t QUEUE_LENGTH = 32;
constexpr int32_t SIZE_COEFFICIENT = 12;
constexpr int32_t TASK_STACK_SIZE = 0x400 * SIZE_COEFFICIENT;
constexpr int32_t APP_TASK_PRI = 25;
SliteAbility *g_NativeAbility = nullptr;
AbilityMgrService *g_AbilityService = nullptr;
AbilityMgrSliteFeature *g_AbilityFeature = nullptr;

AbilityService::AbilityService()
{
}

AbilityService::~AbilityService()
{
    DeleteRecordInfo(LAUNCHER_TOKEN);
}

void AbilityService::StartLauncher()
{
    auto record = new AbilityRecord();
    record->SetAppName(LAUNCHER_BUNDLE_NAME);
    record->SetToken(LAUNCHER_TOKEN);
    record->SetState(SCHEDULE_ACTIVE);
    abilityList_.Add(record);
    abilityStack_.PushAbility(record);
    (void) SchedulerLifecycleInner(record, STATE_ACTIVE);
}

void AbilityService::CleanWant()
{
    ClearWant(want_);
    AdapterFree(want_);
}

int32_t AbilityService::StartAbility(const Want *want)
{
    if (want == nullptr || want->element == nullptr) {
        HILOG_ERROR(HILOG_MODULE_AAFWK, "Ability Service wanted element is null");
        return PARAM_NULL_ERROR;
    }
    char *bundleName = want->element->bundleName;
    if (bundleName == nullptr) {
        HILOG_ERROR(HILOG_MODULE_AAFWK, "Ability Service wanted bundleName is null");
        return PARAM_NULL_ERROR;
    }

    // TODO ����Ѿ���ǰ̨����ֱ�ӷ���
    AbilitySvcInfo *info =
        static_cast<OHOS::AbilitySvcInfo *>(AdapterMalloc(sizeof(AbilitySvcInfo)));
    if (info == nullptr) {
        HILOG_ERROR(HILOG_MODULE_AAFWK, "Ability Service AbilitySvcInfo is null");
        return PARAM_NULL_ERROR;
    }
    // TODO ��ʼ��info

    //  ��װinfo��Ϣ
    //  ���������
    if (strcmp(bundleName, LAUNCHER_BUNDLE_NAME) == 0) {
        info->bundleName = Utils::Strdup(bundleName);
        info->path = nullptr;
    } else {
        // JSӦ��
        AbilityInfo abilityInfo = { nullptr, nullptr };
        QueryAbilityInfo(want, &abilityInfo);
        if ((abilityInfo.bundleName == nullptr) || (strlen(abilityInfo.bundleName) == 0) ||
            (abilityInfo.srcPath == nullptr) || (strlen(abilityInfo.srcPath) == 0)) {
            APP_ERRCODE_EXTRA(EXCE_ACE_APP_START, EXCE_ACE_APP_START_UNKNOWN_BUNDLE_INFO);
            ClearAbilityInfo(&abilityInfo);
            AdapterFree(info);
            HILOG_ERROR(HILOG_MODULE_AAFWK, "Ability Service returned bundleInfo is not valid");
            return PARAM_NULL_ERROR;
        }
        info->bundleName = OHOS::Utils::Strdup(abilityInfo.bundleName);
        info->path = OHOS::Utils::Strdup(abilityInfo.srcPath);
        ClearAbilityInfo(&abilityInfo);
    }

    info->data = OHOS::Utils::Memdup(want->data, want->dataLength);
    info->dataLength = want->dataLength;
    auto ret = StartAbility(info);
    AdapterFree(info->bundleName);
    AdapterFree(info->path);
    AdapterFree(info->data);
    AdapterFree(info);
    return ERR_OK;
}

void AbilityService::UpdataRecord(AbilitySvcInfo *info)
{
    if (info == nullptr) {
        return;
    }
    AbilityRecord *record = abilityList_.Get(info->bundleName);
    if (record == nullptr) {
        return;
    }
    if (record->GetToken() != LAUNCHER_TOKEN) {
        return;
    }
    record->SetAppData(info->data, info->dataLength);
}

int32_t AbilityService::StartAbility(AbilitySvcInfo *info)
{
    if ((info == nullptr) || (info->bundleName == nullptr) || (strlen(info->bundleName) == 0)) {
        return PARAM_NULL_ERROR;
    }
    HILOG_INFO(HILOG_MODULE_AAFWK, "StartAbility");

    auto topRecord = abilityStack_.GetTopAbility();
    if ((topRecord == nullptr) || (topRecord->GetAppName() == nullptr)) {
        HILOG_ERROR(HILOG_MODULE_AAFWK, "StartAbility top null.");
        return PARAM_NULL_ERROR;
    }
    uint16_t topToken = topRecord->GetToken();
    // ������������
    if (strcmp(info->bundleName, LAUNCHER_BUNDLE_NAME) == 0) {
        UpdataRecord(info);
        // Top�������棬����JSӦ���Ҳ��ں�̨
        if (topToken != LAUNCHER_TOKEN && topRecord->GetState() != SCHEDULE_BACKGROUND) {
            HILOG_INFO(HILOG_MODULE_AAFWK, "Change Js app to background.");
            (void) SchedulerLifecycleInner(topRecord, STATE_BACKGROUND);
        } else {
            // TODO �����Ƿ�֪ͨLauncher  TOP�����棬����top�������棬js�ں�̨
            (void) SchedulerLifecycle(LAUNCHER_TOKEN, STATE_ACTIVE);
        }
        return ERR_OK;
    }
    // TODO Check�Ƿ���������
    if (!CheckResponse(info->bundleName)) {
        return -1;
    }

    // ����js��TOP��jsӦ�ã����Ѿ�������task
    if (topRecord->IsAttached() && topRecord->GetToken() != LAUNCHER_TOKEN) {
        // ��֮ǰ�����ͬһ��Ӧ�� tocheck
        if (strcmp(info->bundleName, topRecord->GetAppName()) == 0) {
            if (topRecord->GetState() == SCHEDULE_BACKGROUND) {
                HILOG_INFO(HILOG_MODULE_AAFWK, "StartAbility Resume app when background.");
                (void) SchedulerLifecycle(LAUNCHER_TOKEN, STATE_BACKGROUND);
                return ERR_OK;
            }
            HILOG_INFO(HILOG_MODULE_AAFWK, "Js app already started or starting.");
        } else {
            // TODO ��֮ǰ��jsӦ�ò���һ��Ӧ�� JS2JS��������Ҫ�Ȱ�֮ǰ��JS�˳�
            return ERR_OK;
        }
    }

    // application has not been launched and then to check priority and permission.
    return PreCheckStartAbility(info->bundleName, info->path, info->data, info->dataLength);
}

int32_t AbilityService::TerminateAbility(uint16_t token)
{
    HILOG_INFO(HILOG_MODULE_AAFWK, "TerminateAbility [%u]", token);
    AbilityRecord *topRecord = const_cast<AbilityRecord *>(abilityStack_.GetTopAbility());
    if (topRecord == nullptr) {
        APP_ERRCODE_EXTRA(EXCE_ACE_APP_START, EXCE_ACE_APP_STOP_NO_ABILITY_RUNNING);
        return PARAM_NULL_ERROR;
    }
    uint16_t topToken = topRecord->GetToken();
    // TODO CHECK terminate ����,�Ƿ��иó�����
    if (token == LAUNCHER_TOKEN) {
        // �����ǰjs�ں�̨, �������˺�̨��js�ߵ�ǰ̨
        if (topToken != token && topRecord->GetState() == SCHEDULE_BACKGROUND) {
            HILOG_INFO(HILOG_MODULE_AAFWK, "Resume Js app [%u]", topToken);
            return SchedulerLifecycle(LAUNCHER_TOKEN, STATE_BACKGROUND);
        }
        return ERR_OK;
    }

    // TODO CHECK�Ƿ��иó���
    if (token != topToken) {
        APP_ERRCODE_EXTRA(EXCE_ACE_APP_START, EXCE_ACE_APP_STOP_UNKNOWN_ABILITY_TOKEN);
        DeleteRecordInfo(token);
        return -1;
    }
    topRecord->SetTerminated(true);
    // TerminateAbility TOP��js
    return SchedulerLifecycleInner(topRecord, STATE_BACKGROUND);
}

int32_t AbilityService::PreCheckStartAbility(
    const char *bundleName, const char *path, const void *data, uint16_t dataLength)
{
    if (path == nullptr) {
        HILOG_ERROR(HILOG_MODULE_AAFWK, "PreCheckStartAbility path is null.");
        return PARAM_NULL_ERROR;
    }
    auto curRecord = abilityList_.Get(bundleName);
    if (curRecord != nullptr) {
        // ��ǰ�Ѿ���ǰ̨��������
        if (curRecord->GetState() == SCHEDULE_ACTIVE) {
            HILOG_ERROR(HILOG_MODULE_AAFWK, "PreCheckStartAbility cur state active.");
        } else if (curRecord->GetState() == SCHEDULE_BACKGROUND) {
            // ������js�ں�̨�����Ƚ������ߵ���̨
            SchedulerLifecycle(LAUNCHER_TOKEN, STATE_BACKGROUND);
        }
        return ERR_OK;
    }
    auto record = new AbilityRecord();
    record->SetToken(GenerateToken());
    record->SetAppName(bundleName);
    record->SetAppPath(path);
    record->SetAppData(data, dataLength);
    record->SetState(SCHEDULE_STOP);
    abilityList_.Add(record);
    // ����APP TASK
    if (CreateAppTask(record) != ERR_OK) {
        HILOG_ERROR(HILOG_MODULE_AAFWK, "CheckResponse CreateAppTask fail");
        abilityList_.Erase(record->GetToken());
        delete record;
        return CREATE_APPTASK_ERROR;
    }
    return ERR_OK;
}

bool AbilityService::CheckResponse(const char *bundleName)
{
    return true;
}

int32_t AbilityService::CreateAppTask(AbilityRecord *record)
{
    if ((record == nullptr) || (record->GetAppName() == nullptr)) {
        return PARAM_NULL_ERROR;
    }

    HILOG_INFO(HILOG_MODULE_AAFWK, "CreateAppTask.");
    LOS_TaskLock();
    TSK_INIT_PARAM_S stTskInitParam;
    stTskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)(JsAppHost::JsAppTaskHandler);
    stTskInitParam.uwStackSize = TASK_STACK_SIZE;
    stTskInitParam.usTaskPrio = OS_TASK_PRIORITY_LOWEST - APP_TASK_PRI;
    stTskInitParam.pcName = const_cast<char *>("AppTask");
    stTskInitParam.uwResved = 0;
    auto jsAppHost = new JsAppHost();
    stTskInitParam.uwArg = reinterpret_cast<UINT32>((uintptr_t)jsAppHost);
    UINT32 appTaskId = 0;
    UINT32 res = LOS_TaskCreate(&appTaskId, &stTskInitParam);
    if (res != LOS_OK) {
        APP_ERRCODE_EXTRA(EXCE_ACE_APP_START, EXCE_ACE_APP_START_CREATE_TSAK_FAILED);
        delete jsAppHost;
        LOS_TaskUnlock();
        return CREATE_APPTASK_ERROR;
    }
    osMessageQueueId_t jsAppQueueId = osMessageQueueNew(QUEUE_LENGTH, sizeof(AbilityInnerMsg), nullptr);
    jsAppHost->SetMessageQueueId(jsAppQueueId);
    LOS_TaskUnlock();

    record->SetTaskId(appTaskId);
    record->SetMessageQueueId(jsAppQueueId);
    record->SetJsAppHost(jsAppHost);

    // LoadPermissions(record->GetAppName(), appTaskId)
    record->SetState(SCHEDULE_INACTIVE);
    // TODO ������SchedulerLifecycle֮��
    abilityStack_.PushAbility(record);
    APP_EVENT(MT_ACE_APP_START);
    if (SchedulerLifecycle(LAUNCHER_TOKEN, STATE_BACKGROUND) != 0) {
        APP_ERRCODE_EXTRA(EXCE_ACE_APP_START, EXCE_ACE_APP_START_LAUNCHER_EXIT_FAILED);
        HILOG_INFO(HILOG_MODULE_AAFWK, "CreateAppTask Fail to hide launcher");
        abilityStack_.PopAbility();
        return SCHEDULER_LIFECYCLE_ERROR;
    }
    return ERR_OK;
}

uint16_t AbilityService::GenerateToken()
{
    static uint16_t token = LAUNCHER_TOKEN;
    if (token == UINT16_MAX - 1) {
        token = LAUNCHER_TOKEN;
    }
    return ++token;
}

void AbilityService::DeleteRecordInfo(uint16_t token)
{
    AbilityRecord *record = abilityList_.Get(token);
    if (record == nullptr) {
        return;
    }
    if (token != LAUNCHER_TOKEN) {
        if (record->IsAttached()) {
            UINT32 taskId = record->GetTaskId();
            // UnLoadPermissions(taskId)
            LOS_TaskDelete(taskId);
            osMessageQueueId_t jsAppQueueId = record->GetMessageQueueId();
            osMessageQueueDelete(jsAppQueueId);
            auto jsAppHost = const_cast<JsAppHost *>(record->GetJsAppHost());
            delete jsAppHost;
            // free all JS native memory after exiting it
            // CleanTaskMem(taskId)
        }
        // record app info event when stop app
        RecordAbiityInfoEvt(record->GetAppName());
    }
    abilityList_.Erase(token);
    delete record;
}

void AbilityService::OnActiveDone(uint16_t token)
{
    HILOG_INFO(HILOG_MODULE_AAFWK, "OnActiveDone [%u]", token);
    SetAbilityState(token, SCHEDULE_ACTIVE);
    auto topRecord = const_cast<AbilityRecord *>(abilityStack_.GetTopAbility());
    if (topRecord == nullptr) {
        return;
    }
    if (token == LAUNCHER_TOKEN) {
        // ��������漤���JS��״̬��Ϊ��̨�����˳�JS
        if (topRecord->GetToken() != LAUNCHER_TOKEN) {
            if (topRecord->GetState() != SCHEDULE_BACKGROUND) {
                APP_ERRCODE_EXTRA(EXCE_ACE_APP_START, EXCE_ACE_APP_START_LAUNCHER_EXIT_FAILED);
                HILOG_ERROR(HILOG_MODULE_AAFWK, "Active launcher js bg fail");
                abilityStack_.PopAbility();
                DeleteRecordInfo(topRecord->GetToken());
            } else if (topRecord->IsTerminated()) {
                (void) SchedulerLifecycleInner(topRecord, STATE_UNINITIALIZED);
            }
        }
        return;
    }
    // �����������js
    if (topRecord->GetToken() == token) {
        APP_EVENT(MT_ACE_APP_ACTIVE);
    }
}

void AbilityService::OnBackgroundDone(uint16_t token)
{
    HILOG_INFO(HILOG_MODULE_AAFWK, "OnBackgroundDone [%u]", token);
    SetAbilityState(token, SCHEDULE_BACKGROUND);
    auto topRecord = const_cast<AbilityRecord *>(abilityStack_.GetTopAbility());
    if (topRecord == nullptr) {
        return;
    }
    // JS�ߵ���̨������LAUNCHER
    if (token != LAUNCHER_TOKEN) {
        if (topRecord->GetToken() == token) {
            APP_EVENT(MT_ACE_APP_BACKGROUND);
            topRecord->SetTerminated(true);
            (void) SchedulerLifecycle(LAUNCHER_TOKEN, STATE_ACTIVE);
        }
        return;
    }
    // launcher�ߵ���̨����
    if (topRecord->GetToken() != LAUNCHER_TOKEN) {
        (void) SchedulerLifecycleInner(topRecord, STATE_ACTIVE);
    } else {
        // TODO CHECK�����������û��
        HILOG_WARN(HILOG_MODULE_AAFWK, "Js app exit, but has no js app.");
    }
}

void AbilityService::OnDestroyDone(uint16_t token)
{
    HILOG_INFO(HILOG_MODULE_AAFWK, "OnDestroyDone [%u]", token);
    // ���治��destroy
    if (token == LAUNCHER_TOKEN) {
        SetAbilityState(token, SCHEDULE_STOP);
        return;
    }
    auto topRecord = abilityStack_.GetTopAbility();
    if ((topRecord == nullptr) || (topRecord->GetToken() != token)) {
        SetAbilityState(token, SCHEDULE_STOP);
        DeleteRecordInfo(token);
        return;
    }
    APP_EVENT(MT_ACE_APP_STOP);
    abilityStack_.PopAbility();
    DeleteRecordInfo(token);
    SetAbilityState(token, SCHEDULE_STOP);

    // ������JS��Ҫ�����ĳ�����ֱ�Ӽ�������
    if (pendingToken_ == 0) {
        (void) SchedulerLifecycle(LAUNCHER_TOKEN, STATE_ACTIVE);
        return;
    }

    // JS2JS����
    auto record = abilityList_.Get(pendingToken_);
    if (CreateAppTask(record) != ERR_OK) {
        abilityList_.Erase(pendingToken_);
        delete record;
        (void) SchedulerLifecycle(LAUNCHER_TOKEN, STATE_ACTIVE);
    }
    pendingToken_ = 0;
}

int32_t AbilityService::SchedulerLifecycle(uint64_t token, int32_t state)
{
    AbilityRecord *record = abilityList_.Get(token);
    if (record == nullptr) {
        return PARAM_NULL_ERROR;
    }
    return SchedulerLifecycleInner(record, state);
}

void AbilityService::SetAbilityState(uint64_t token, int32_t state)
{
    AbilityRecord *record = abilityList_.Get(token);
    if (record == nullptr) {
        return;
    }
    record->SetState((AbilityState) state);
}

int32_t AbilityService::SchedulerLifecycleInner(const AbilityRecord *record, int32_t state)
{
    if (record == nullptr) {
        return PARAM_NULL_ERROR;
    }
    // ֪ͨ��Ӧ��ability������Ӧ����������
    if (record->GetToken() != LAUNCHER_TOKEN) {  // jsӦ��
        (void) SendMsgToJsAbility(state, record);
        return ERR_OK;
    }
    // native Ӧ�ã���ȡnative��ability��������Ӧ����������
    if (g_NativeAbility == nullptr) {
        return PARAM_NULL_ERROR;
    }
    // ����want�ڴ棬�ڷ���������ͷ�
    Want *info = static_cast<Want *>(AdapterMalloc(sizeof(Want)));
    if (info == nullptr) {
        return MEMORY_MALLOC_ERROR;
    }
    info->element = nullptr;
    info->data = nullptr;
    info->dataLength = 0;

    ElementName elementName = {};
    SetElementBundleName(&elementName, LAUNCHER_BUNDLE_NAME);
    SetWantElement(info, elementName);
    ClearElement(&elementName);
    SetWantData(info, record->GetAppData(), record->GetDataLength());
    SchedulerAbilityLifecycle(g_NativeAbility, *info, state);
    ClearWant(info);
    return ERR_OK;
}

void AbilityService::SchedulerAbilityLifecycle(SliteAbility *ability, const Want &want, int32_t state)
{
    if (ability == nullptr) {
        return;
    }
    switch (state) {
        case STATE_ACTIVE: {
            ability->OnActive(want);
            break;
        }
        case STATE_BACKGROUND: {
            ability->OnBackground();
            break;
        }
        default: {
            break;
        }
    }
    return;
}

int32_t AbilityService::SchedulerLifecycleDone(uint64_t token, int32_t state)
{
    switch (state) {
        case STATE_ACTIVE: {
            OnActiveDone(token);
            break;
        }
        case STATE_BACKGROUND: {
            OnBackgroundDone(token);
            break;
        }
        case STATE_UNINITIALIZED: {  // TODO DESTROY�������
            OnDestroyDone(token);
            break;
        }
        default: {
            break;
        }
    }
    return ERR_OK;
}

bool AbilityService::SendMsgToJsAbility(int32_t state, const AbilityRecord *record)
{
    if (record == nullptr) {
        return false;
    }

    AbilityInnerMsg innerMsg;
    if (state == STATE_ACTIVE) {
        innerMsg.msgId = ACTIVE;
    } else if (state == STATE_BACKGROUND) {
        innerMsg.msgId = BACKGROUND;
    } else if (state == STATE_UNINITIALIZED) {
        innerMsg.msgId = DESTORY;
    } else {
        innerMsg.msgId = (AbilityMsgId) state;
    }
    innerMsg.bundleName = record->GetAppName();
    innerMsg.token = record->GetToken();
    innerMsg.path = record->GetAppPath();
    innerMsg.data = const_cast<void *>(record->GetAppData());
    innerMsg.dataLength = record->GetDataLength();
    osMessageQueueId_t appQueueId = record->GetMessageQueueId();
    osStatus_t ret = osMessageQueuePut(appQueueId, static_cast<void *>(&innerMsg), 0, 0);
    return ret == osOK;
}

void AbilityService::setNativeAbility(const SliteAbility *ability)
{
    g_AbilityFeature = AbilityMgrSliteFeature::GetInstance();
    g_AbilityService = AbilityMgrService::GetInstance();
    g_NativeAbility = const_cast<SliteAbility *>(ability);
}
} // namespace OHOS

extern "C" {
int InstallNativeAbility(const AbilityInfo *abilityInfo, const OHOS::SliteAbility *ability)
{
    OHOS::AbilityService::GetInstance().setNativeAbility(ability);
    return ERR_OK;
}
}