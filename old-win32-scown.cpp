#include <windows.h>
#include <iostream>

std::string GetLastErrorAsString();

int main(int argc, char const *argv[])
{
    DISPLAY_DEVICE display_adaptor = {};
    display_adaptor.cb = sizeof(DISPLAY_DEVICE);
    DEVMODE adapter_info = {};
    adapter_info.dmSize = sizeof(DEVMODE);

    for(
        unsigned adaptor_idx = 0;
        EnumDisplayDevices(NULL, adaptor_idx, &display_adaptor, 0);
        adaptor_idx++
    ) {
        DISPLAY_DEVICE monitor = {};
        monitor.cb = sizeof(DISPLAY_DEVICE);
        DEVMODE monitor_mode = {};
        monitor_mode.dmSize = sizeof(DEVMODE);
        
        for(
            unsigned monitor_idx = 0;
            EnumDisplayDevices(display_adaptor.DeviceName, monitor_idx, &monitor, 0);
            monitor_idx++
        ) {
            if (monitor_idx == 0) {
                std::cout << "Display apapter: " << display_adaptor.DeviceString;
                std::cout << " (" << display_adaptor.DeviceName << ")" << std::endl;
            }
            std::cout << "- monitor: " << monitor.DeviceString;
            std::cout << " (" << monitor.DeviceName << ")" << std::endl;

            bool attached = monitor.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;
            bool primary = monitor.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE;
            if (attached) {
                std::cout << "   - attached!" << std::endl;
            } else  {
                std::cout << "   - detached!" << std::endl;
            }
            if (primary) {
                std::cout << "   - primary!" << std::endl;
            }

            ;

            if (0 == EnumDisplaySettings(display_adaptor.DeviceName, ENUM_CURRENT_SETTINGS, &monitor_mode)) {
                std::cout << "   - current mode unavailable!" << std::endl;
                std::cout << "Error while readind display settings: " << GetLastErrorAsString() << std::endl;
            } else {
                std::cout << "   - current mode: ";
                std::cout << monitor_mode.dmPelsWidth << "x";
                std::cout << monitor_mode.dmPelsHeight << "x";
                std::cout << monitor_mode.dmBitsPerPel << "@";
                std::cout << monitor_mode.dmDisplayFrequency << std::endl;
            }

            long detach_success = 0;
            if (attached && !primary) {
                std::cout << "Attempting to detach display!" << std::endl;
                DEVMODE detach_mode = {};
                detach_mode.dmSize = sizeof(DEVMODE);
                detach_mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION
                                    | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS ;
                detach_success = ChangeDisplaySettingsEx(display_adaptor.DeviceName, &detach_mode, NULL, CDS_UPDATEREGISTRY, NULL);
                ChangeDisplaySettingsEx (NULL, NULL, NULL, NULL, NULL);
            } else {
                detach_success = ChangeDisplaySettingsEx(display_adaptor.DeviceName, 0, NULL, CDS_UPDATEREGISTRY, NULL);
                ChangeDisplaySettingsEx (NULL, NULL, NULL, NULL, NULL);
            }
            
            switch(detach_success) {
            case DISP_CHANGE_BADDUALVIEW:
                std::cout << "Error: Change unsuccessful because system is DualView capable." << std::endl;
                break;
            case DISP_CHANGE_BADFLAGS:
                std::cout << "Error: Change unsuccessful because of invalid flags." << std::endl;
                break;
            case DISP_CHANGE_BADPARAM:
                std::cout << "Error: Change unsuccessful because of invalid parameters passed in." << std::endl;
                break;
            case DISP_CHANGE_BADMODE:
                std::cout << "Error: Change unsuccessful because mode not supported." << std::endl;
                break;
            case DISP_CHANGE_FAILED:
                std::cout << "Error: Change unsuccessful because display driver failed." << std::endl;
                break;
            case DISP_CHANGE_NOTUPDATED:
                std::cout << "Error: Unable to write to registry." << std::endl;
                break;
            case DISP_CHANGE_RESTART:
                std::cout << "Error: Computer must be restarted to make changes." << std::endl;
                break;
            case DISP_CHANGE_SUCCESSFUL:
                break;
            default:
                std::cout << "Unknown error: Change unsuccessful." << std::endl;
                break;
            }
        }
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