#include "vps.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VPS w;
    w.show();
    return a.exec();
}

/*NOTES ON JSON:

Data op:  7
    QJsonDocument({"d":{"requestId":"N/A","requestStatus":{"code":100,"result":true},"requestType":"GetInputKindList","responseData":{"inputKinds":["image_source","color_source_v3","slideshow_v2","browser_source","ffmpeg_source","text_gdiplus_v3","text_ft2_source_v2","vlc_source","monitor_capture","window_capture","game_capture","dshow_input","wasapi_input_capture","wasapi_output_capture","wasapi_process_output_capture"]}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"N/A","requestStatus":{"code":100,"result":true},"requestType":"GetInputList","responseData":{"inputs":[{"inputKind":"wasapi_input_capture","inputName":"Mic/Aux","inputUuid":"24add2a9-78b3-4c63-83d6-d3ffe5c63258","unversionedInputKind":"wasapi_input_capture"},{"inputKind":"slideshow_v2","inputName":"VPS Slideshow","inputUuid":"a23625d6-2588-4a74-81da-13073868fe49","unversionedInputKind":"slideshow"},{"inputKind":"wasapi_input_capture","inputName":"Audio Input Capture","inputUuid":"dd788641-4129-449b-8cd0-919728dd6c37","unversionedInputKind":"wasapi_input_capture"},{"inputKind":"ffmpeg_source","inputName":"Media Source","inputUuid":"fc4f1528-3a0e-474a-9293-d851a5449c43","unversionedInputKind":"ffmpeg_source"},{"inputKind":"wasapi_input_capture","inputName":"Audio Input Capture 1","inputUuid":"9f0de2fe-091f-4829-ade8-56c52981227c","unversionedInputKind":"wasapi_input_capture"},{"inputKind":"wasapi_input_capture","inputName":"Audio Input Capture 2","inputUuid":"c6f79354-5af7-46b7-894f-747663bcd950","unversionedInputKind":"wasapi_input_capture"},{"inputKind":"ffmpeg_source","inputName":"Media Source Quack","inputUuid":"5d94fec9-e482-4755-96bc-a2b972bdf5e3","unversionedInputKind":"ffmpeg_source"},{"inputKind":"ffmpeg_source","inputName":"SFX","inputUuid":"bd9fe190-cf7a-453f-8bdd-f63308fa0441","unversionedInputKind":"ffmpeg_source"},{"inputKind":"color_source_v3","inputName":"Color Green","inputUuid":"ec290547-b40e-40fc-9335-fa3e4ac128ef","unversionedInputKind":"color_source"},{"inputKind":"color_source_v3","inputName":"Color Red","inputUuid":"53b7e4f7-f3d5-47b0-a2dd-2414611acf51","unversionedInputKind":"color_source"},{"inputKind":"dshow_input","inputName":"Video Capture Device","inputUuid":"07a0fde4-ff33-47b5-810c-b4262b90373e","unversionedInputKind":"dshow_input"}]}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"VCAP","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"dshow_input","inputSettings":{"audio_device_id":"HDMI (USB Capture HDMI 4K+):","last_video_device_id":"USB Capture HDMI 4K+:\\\\?\\usb#22vid_2935&pid_0009&mi_00#229&31575d26&0&0000#22{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\global","undo_uuid":"07a0fde4-ff33-47b5-810c-b4262b90373e","use_custom_audio_device":true,"video_device_id":"USB Capture HDMI 4K+:\\\\?\\usb#22vid_2935&pid_0009&mi_00#229&31575d26&0&0000#22{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\global"}}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"Special","requestStatus":{"code":100,"result":true},"requestType":"GetSpecialInputs","responseData":{"desktop1":null,"desktop2":null,"mic1":"Mic/Aux","mic2":null,"mic3":null,"mic4":null}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"MEDIASRC","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"ffmpeg_source","inputSettings":{"local_file":"C:/Users/Broadcasting/Downloads/Test1080p30s.mp4"}}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"AUDIOPROPER","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"wasapi_input_capture","inputSettings":{"device_id":"{0.0.1.00000000}.{9e556905-d2b5-4d7a-8b73-0e5f34606669}"}}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"AUDIOBADINPUT","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"wasapi_input_capture","inputSettings":{"device_id":"{0.0.1.00000000}.{cc384159-7719-4f71-aa82-49b2196df288}"}}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"CFGSLIDESHOW","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"slideshow_v2","inputSettings":{"files":[{"hidden":false,"selected":false,"uuid":"f7989fcb-8a18-4e85-b81c-0ac9b605ba17","value":"C:/Users/Broadcasting/Desktop/Announcements Media/LIVE Slide Rotation/Test/Blue.png"},{"hidden":false,"selected":false,"uuid":"ed744b03-2628-4f3b-8e8b-409651c9c674","value":"C:/Users/Broadcasting/Desktop/Announcements Media/LIVE Slide Rotation/Test/Green.png"},{"hidden":false,"selected":false,"uuid":"8d83aae4-5ff9-45e1-88ec-35c2d25f85fd","value":"C:/Users/Broadcasting/Desktop/Announcements Media/LIVE Slide Rotation/Test/Red.png"},{"hidden":false,"selected":false,"uuid":"fd9ca88c-c41d-4018-9abf-7ff228663739","value":"C:/Users/Broadcasting/Desktop/Announcements Media/LIVE Slide Rotation/Test/Blue.png"},{"hidden":false,"selected":false,"uuid":"85a1e21f-0fe5-4ace-a9e4-a162cec483ca","value":"C:/Users/Broadcasting/Desktop/Announcements Media/LIVE Slide Rotation/Test/Green.png"},{"hidden":false,"selected":false,"uuid":"904af022-9370-4e75-82e1-987d6a0a0eab","value":"C:/Users/Broadcasting/Desktop/Announcements Media/LIVE Slide Rotation/Test/Red.png"}],"slide_time":10000,"transition_speed":300}}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"AUDIOBADDEFAULT","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"wasapi_input_capture","inputSettings":{}}},"op":7})
    Data op:  7
    QJsonDocument({"d":{"requestId":"CFGQUACK","requestStatus":{"code":100,"result":true},"requestType":"GetInputSettings","responseData":{"inputKind":"ffmpeg_source","inputSettings":{"close_when_inactive":false,"local_file":"C:/Users/Broadcasting/Downloads/quack_5.mp3","looping":false}}},"op":7})
    Data op:  5
    QJsonDocument({"d":{"eventData":{"sceneItemId":2,"sceneName":"VPS Reference","sceneUuid":"37d359c7-eb5a-45da-a102-cf4c083029e3"},"eventIntent":128,"eventType":"SceneItemSelected"},"op":5})
    Data op:  5
    QJsonDocument({"d":{"eventData":{"sceneItemId":3,"sceneName":"VPS Reference","sceneUuid":"37d359c7-eb5a-45da-a102-cf4c083029e3"},"eventIntent":128,"eventType":"SceneItemSelected"},"op":5})
    Data op:  5
    QJsonDocument({"d":{"eventData":{"sceneItemId":4,"sceneName":"VPS Reference","sceneUuid":"37d359c7-eb5a-45da-a102-cf4c083029e3"},"eventIntent":128,"eventType":"SceneItemSelected"},"op":5})
    
    
    QJsonDocument({"d":{"requestId":"GET_SCENE_ITEMS_INFO","requestStatus":{"code":100,"result":true},"requestType":"GetSceneItemList","responseData":{"sceneItems":[{"inputKind":"color_source_v3","isGroup":null,"sceneItemBlendMode":"OBS_BLEND_NORMAL","sceneItemEnabled":true,"sceneItemId":1,"sceneItemIndex":0,"sceneItemLocked":false,"sceneItemTransform":{"alignment":5,"boundsAlignment":0,"boundsHeight":0,"boundsType":"OBS_BOUNDS_NONE","boundsWidth":0,"cropBottom":0,"cropLeft":0,"cropRight":0,"cropToBounds":false,"cropTop":0,"height":1080,"positionX":0,"positionY":0,"rotation":0,"scaleX":1,"scaleY":1,"sourceHeight":1080,"sourceWidth":1920,"width":1920},"sourceName":"Blue","sourceType":"OBS_SOURCE_TYPE_INPUT","sourceUuid":"eeb7a78f-3f43-4160-ab66-b86490f9c18f"},{"inputKind":"color_source_v3","isGroup":null,"sceneItemBlendMode":"OBS_BLEND_NORMAL","sceneItemEnabled":true,"sceneItemId":2,"sceneItemIndex":1,"sceneItemLocked":false,"sceneItemTransform":{"alignment":5,"boundsAlignment":0,"boundsHeight":0,"boundsType":"OBS_BOUNDS_NONE","boundsWidth":0,"cropBottom":0,"cropLeft":0,"cropRight":0,"cropToBounds":false,"cropTop":0,"height":1080,"positionX":0,"positionY":0,"rotation":0,"scaleX":1,"scaleY":1,"sourceHeight":1080,"sourceWidth":1920,"width":1920},"sourceName":"Red","sourceType":"OBS_SOURCE_TYPE_INPUT","sourceUuid":"3c8c777f-2765-454d-b9c6-75a198a522e8"},{"inputKind":"color_source_v3","isGroup":null,"sceneItemBlendMode":"OBS_BLEND_NORMAL","sceneItemEnabled":true,"sceneItemId":3,"sceneItemIndex":2,"sceneItemLocked":false,"sceneItemTransform":{"alignment":5,"boundsAlignment":0,"boundsHeight":0,"boundsType":"OBS_BOUNDS_NONE","boundsWidth":0,"cropBottom":0,"cropLeft":0,"cropRight":0,"cropToBounds":false,"cropTop":0,"height":1080,"positionX":0,"positionY":0,"rotation":0,"scaleX":1,"scaleY":1,"sourceHeight":1080,"sourceWidth":1920,"width":1920},"sourceName":"Green","sourceType":"OBS_SOURCE_TYPE_INPUT","sourceUuid":"55741d4c-cb50-4356-b6c9-55c6353b8b8f"}]}},"op":7})

*/
