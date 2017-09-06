#ifndef FSEditor_H_
#define FSEditor_H_
#include <ESPAsyncWebServer.h>
class FSEditor: public AsyncWebHandler {
  private:
    String _username;
    String _password;
    bool _authenticated;
    uint32_t _startTime;
  public:
    FSEditor(const String& username=String(), const String& password=String());
    virtual bool canHandle(AsyncWebServerRequest *request) override final;
    virtual void handleRequest(AsyncWebServerRequest *request) override final;
    virtual void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) override final;
};
#endif
