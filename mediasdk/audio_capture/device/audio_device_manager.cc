#include "audio_device_manager.h"

#include <comdef.h>
#include <dmo.h>
#include <functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <mmsystem.h>
#include <strsafe.h>
#include <uuids.h>
#include <windows.h>

#include "string_utils.h"

AudioDeviceManager& AudioDeviceManager::GetInstance() {
    static AudioDeviceManager instance;
    return instance;
}

AudioDeviceManager::AudioDeviceManager() {
    Init();
}

AudioDeviceManager::~AudioDeviceManager() {
    Uninit();
}

AudioDeviceInfo AudioDeviceManager::GetDefaultPlayoutAudioDevice() {
    if (!init_) {
        return AudioDeviceInfo();
    }
    EDataFlow flow = eRender;
    return GetDefaultAudioDevice(flow);
}

AudioDeviceInfo AudioDeviceManager::GetDefaultRecordAudioDevice() {
    if (!init_) {
        return AudioDeviceInfo();
    }
    EDataFlow flow = eCapture;
    return GetDefaultAudioDevice(flow);
}

AudioDeviceInfo AudioDeviceManager::GetDefaultAudioDevice(EDataFlow flow) {
    IMMDeviceCollection* collection = nullptr;
    IMMDevice* endpoint = nullptr;;
    ERole role = eConsole;
    enumerator_->GetDefaultAudioEndpoint(flow, eConsole, &endpoint);
    AudioDeviceInfo audio_device = GetDevice(endpoint);
    SAFE_RELEASE(endpoint);
    return audio_device;
}

std::vector<AudioDeviceInfo> AudioDeviceManager::EnumPlayoutAudioDevices() {
    if (!init_) {
        return std::vector<AudioDeviceInfo>();
    }
    EDataFlow flow = eRender;
    return GetAudioDevice(flow);
}

std::vector<AudioDeviceInfo> AudioDeviceManager::EnumRecordAudioDevices() {
    if (!init_) {
        return std::vector<AudioDeviceInfo>();
    }
    EDataFlow flow = eCapture;
    return GetAudioDevice(flow);
}

std::vector<AudioDeviceInfo> AudioDeviceManager::GetAudioDevice(EDataFlow flow) {
    IMMDeviceCollection* collection = nullptr;
    if (flow == eRender) {
        collection = render_collection_;
    } else if (flow == eCapture) {
        collection = capture_collection_;
    }
    if (!collection) {
        return std::vector<AudioDeviceInfo>();
    }
    std::vector<AudioDeviceInfo> result;
    UINT count = 0;
    collection->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        IMMDevice* pDevice = NULL;
        collection->Item(i, &pDevice);
        AudioDeviceInfo audio_device = GetDevice(pDevice);
        if (!audio_device.device_id.empty()) {
            result.push_back(std::move(audio_device));
        }
        SAFE_RELEASE(pDevice);
    }
    return result;
}

AudioDeviceInfo AudioDeviceManager::GetDevice(IMMDevice* pDevice) {
    AudioDeviceInfo device;
    if (!pDevice) {
        return device;
    }
    LPWSTR pwszID = NULL;
    HRESULT hr = pDevice->GetId(&pwszID);
    // _TRUNCATE
    device.device_id = utils::UnicodeToUtf8(pwszID);
    IPropertyStore* pProps = NULL;
    PROPVARIANT varName;
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (FAILED(hr)) {
        return device;
    }
    PropVariantInit(&varName);
    hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    if (SUCCEEDED(hr)) {
        device.device_name = utils::UnicodeToUtf8(varName.pwszVal);
    }
    return device;
}

bool AudioDeviceManager::Init() {
    // CoInitialize(NULL);
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr =
        CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                         __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator_));
    if (!enumerator_) {
        return false;
    }
    hr = enumerator_->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &render_collection_);
    if (hr != S_OK || !render_collection_) {
        return false;
    }
    hr = enumerator_->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &capture_collection_);
    if (hr != S_OK || !capture_collection_) {
        return false;
    }
    init_ = true;
    return true;
}

bool AudioDeviceManager::Uninit() {
    CoUninitialize();
    if (enumerator_) {
        SAFE_RELEASE(enumerator_);
    }
    if (render_collection_) {
        SAFE_RELEASE(render_collection_);
    }
    if (capture_collection_) {
        SAFE_RELEASE(capture_collection_);
    }
    return true;
}