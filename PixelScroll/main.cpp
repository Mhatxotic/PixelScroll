#include <ApplicationServices/ApplicationServices.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define SCALE 1
#define BUTTON 5

CGEventRef cgEventCallback(CGEventTapProxy cgetpProxy, CGEventType cgetType,
                             CGEventRef cgerEvent, void*const vpRefPtr) {
  static bool bEnabled = false;
  static CGPoint cgpSaved;
  switch(cgetType) {
    case kCGEventOtherMouseDown:
      if(static_cast<int32_t>(CGEventGetIntegerValueField(cgerEvent,
           kCGMouseEventButtonNumber)) == BUTTON) {
        bEnabled = !bEnabled;
        cgpSaved = CGEventGetLocation(cgerEvent);
        if (bEnabled) {
          CGSetLocalEventsSuppressionInterval(10.0);
          CGWarpMouseCursorPosition(cgpSaved);
        }
        else {
          CGSetLocalEventsSuppressionInterval(0.0);
          CGWarpMouseCursorPosition(cgpSaved);
          CGSetLocalEventsSuppressionInterval(0.25);
        }
        return nullptr;
      }
      break;
    case kCGEventMouseMoved:
      if(bEnabled) {
        int32_t iDeltaX =
          static_cast<int32_t>(CGEventGetIntegerValueField(cgerEvent,
            kCGMouseEventDeltaX)),
          iDeltaY =
            static_cast<int32_t>(CGEventGetIntegerValueField(cgerEvent,
              kCGMouseEventDeltaY));
        if(CGEventSourceRef cgesrSource =
             CGEventSourceCreate(kCGEventSourceStateCombinedSessionState)) {
          if(CGEventRef cgerScroll = CGEventCreateScrollWheelEvent(cgesrSource,
               kCGScrollEventUnitPixel, 2,
                 -SCALE * iDeltaY, -SCALE * iDeltaX)) {
            CGEventPost(kCGSessionEventTap, cgerScroll);
            CFRelease(cgerScroll);
          }
          CFRelease(cgesrSource);
        }
        CGWarpMouseCursorPosition(cgpSaved);
      }
      break;
    default: break;
  }
  return cgerEvent;
}

int main(void) {
  if(CGEventMask cgemMask = CGEventMaskBit(kCGEventMouseMoved) |
       CGEventMaskBit(kCGEventOtherMouseDown)) {
    if(CFMachPortRef cfmprTap = CGEventTapCreate(kCGSessionEventTap,
         kCGHeadInsertEventTap, static_cast<CGEventTapOptions>(0), cgemMask,
         cgEventCallback, nullptr)) {
      if(CFRunLoopSourceRef cfrlsrSource =
           CFMachPortCreateRunLoopSource(kCFAllocatorDefault, cfmprTap, 0)) {
        CFRunLoopAddSource(CFRunLoopGetCurrent(), cfrlsrSource,
          kCFRunLoopCommonModes);
        CGEventTapEnable(cfmprTap, true);
        CFRunLoopRun();
        CFRelease(cfrlsrSource);
      }
      CFRelease(cfmprTap);
    }
  }
  return 0;
}
