#include <windows.h>
#include <iostream>
#include <vector>

std::string GetLastErrorAsString();

int main(int argc, char const *argv[])
{
    unsigned paths, modes;
    int result = GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &paths, &modes);
    switch (result) {
    case ERROR_SUCCESS:
        break;
    case ERROR_INVALID_PARAMETER:
        std::cout << "Error: Invalid parameters to GetDisplayConfigBufferSizes()." << std::endl;
        return -1;
    case ERROR_NOT_SUPPORTED:
        std::cout << "Error: Graphics driver not supported." << std::endl;
        return -1;
    case ERROR_ACCESS_DENIED:
        std::cout << "Error: Caller does not have access to the current desktop." << std::endl;
        return -1;
    case ERROR_GEN_FAILURE:
    default:
        std::cout << "Unknown error: Could not GetDisplayConfigBufferSizes()." << std::endl;
        return -1;
    }
    
    std::vector<DISPLAYCONFIG_PATH_INFO> display_paths(paths);
    std::vector<DISPLAYCONFIG_MODE_INFO> display_modes(modes);

    result = QueryDisplayConfig(QDC_ALL_PATHS,
                                &paths, display_paths.data(),
                                &modes, display_modes.data(),
                                NULL);
    
    switch (result) {
    case ERROR_SUCCESS:
        break;
    case ERROR_INVALID_PARAMETER:
        std::cout << "Error: Invalid parameters to QueryDisplayConfig()." << std::endl;
        return -1;
    case ERROR_INSUFFICIENT_BUFFER:
        std::cout << "Error: Suppied buffers are too small." << std::endl;
        return -1;
    case ERROR_NOT_SUPPORTED:
        std::cout << "Error: Graphics driver not supported." << std::endl;
        return -1;
    case ERROR_ACCESS_DENIED:
        std::cout << "Error: Caller does not have access to the current desktop." << std::endl;
        return -1;
    case ERROR_GEN_FAILURE:
    default:
        std::cout << "Unknown error: Could not QueryDisplayConfig()." << std::endl;
        return -1;
    }
    
    bool detached = false;

    for (auto& path : display_paths) {
        if (path.sourceInfo.modeInfoIdx == DISPLAYCONFIG_PATH_MODE_IDX_INVALID &&
            path.targetInfo.modeInfoIdx == DISPLAYCONFIG_PATH_MODE_IDX_INVALID)
            continue;
        


        if (path.sourceInfo.modeInfoIdx == DISPLAYCONFIG_PATH_MODE_IDX_INVALID)
            std::cout << "Target: " << path.targetInfo.modeInfoIdx << std::endl;
        else if (path.targetInfo.modeInfoIdx == DISPLAYCONFIG_PATH_MODE_IDX_INVALID)
            std::cout << "Source: " << path.sourceInfo.modeInfoIdx << std::endl;
        else {
            std::cout << "Source: " << path.sourceInfo.modeInfoIdx;
            std::cout << " --> target: " << path.targetInfo.modeInfoIdx << std::endl;
        }

        DISPLAYCONFIG_TARGET_DEVICE_NAME name_request;
        name_request.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        name_request.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
        name_request.header.adapterId = path.targetInfo.adapterId;
        name_request.header.id = path.targetInfo.id;
        result = DisplayConfigGetDeviceInfo(&name_request.header);

        switch (result) {
        case ERROR_SUCCESS:
            break;
        case ERROR_INVALID_PARAMETER:
            std::cout << "Error: Invalid parameters to DisplayConfigGetDeviceInfo()." << std::endl;
            return -1;
        case ERROR_INSUFFICIENT_BUFFER:
            std::cout << "Error: Suppied buffer is too small." << std::endl;
            return -1;
        case ERROR_NOT_SUPPORTED:
            std::cout << "Error: Graphics driver not supported." << std::endl;
            return -1;
        case ERROR_ACCESS_DENIED:
            std::cout << "Error: Caller does not have access to the current desktop." << std::endl;
            return -1;
        case ERROR_GEN_FAILURE:
        default:
            std::cout << "Unknown error: Could not DisplayConfigGetDeviceInfo()." << std::endl;
            return -1;
        }
        
        std::wcout << L"Monitor: " << name_request.monitorFriendlyDeviceName << std::endl;
        
        if (!detached && path.sourceInfo.modeInfoIdx != DISPLAYCONFIG_PATH_MODE_IDX_INVALID) {
            std::cout << "-- detach!" << std::endl;
            path.flags = 0;  // Detach
            detached = true;
        }
    }

    auto flags = SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_ALLOW_CHANGES;
    result = SetDisplayConfig(paths, display_paths.data(), modes, display_modes.data(), flags);
    
    switch (result) {
    case ERROR_SUCCESS:
        break;
    case ERROR_INVALID_PARAMETER:
        std::cout << "Error: Invalid parameters to SetDisplayConfig()." << std::endl;
        return -1;
    case ERROR_BAD_CONFIGURATION:
        std::cout << "Error: Bad configuration." << std::endl;
        return -1;
    case ERROR_NOT_SUPPORTED:
        std::cout << "Error: Graphics driver not supported." << std::endl;
        return -1;
    case ERROR_ACCESS_DENIED:
        std::cout << "Error: Caller does not have access to the current desktop." << std::endl;
        return -1;
    case ERROR_GEN_FAILURE:
    default:
        std::cout << "Unknown error: Could not DisplayConfigGetDeviceInfo()." << std::endl;
        return -1;
    }

    return 0;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message;
}