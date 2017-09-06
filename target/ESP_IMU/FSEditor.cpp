#include "FSEditor.h"
#include <FS.h>

FSEditor::FSEditor(const String& username, const String& password)
:_username(username)
,_password(password)
,_authenticated(false)
,_startTime(0)
{}

bool FSEditor::canHandle(AsyncWebServerRequest *request){
  if(request->url().equalsIgnoreCase("/edit")){
    if(request->method() == HTTP_GET){
      if(request->hasParam("list"))
        return true;
      if(request->hasParam("edit")){
        request->_tempFile = SPIFFS.open(request->arg("edit"), "r");
        if(!request->_tempFile)
          return false;
      }
      if(request->hasParam("download")){
        request->_tempFile = SPIFFS.open(request->arg("download"), "r");
        if(!request->_tempFile)
          return false;
      }
      return true;
    }
    else if(request->method() == HTTP_POST)
      return true;
    else if(request->method() == HTTP_DELETE)
      return true;
    else if(request->method() == HTTP_PUT)
      return true;

  }
  return false;
}

void FSEditor::handleRequest(AsyncWebServerRequest *request){
  if(_username.length() && _password.length() && !request->authenticate(_username.c_str(), _password.c_str()))
    return request->requestAuthentication();

  if(request->method() == HTTP_GET){
    if(request->hasParam("list")){
      String path = request->getParam("list")->value();
      Dir dir = SPIFFS.openDir(path);
      path = String();
      String output = "[";
      while(dir.next()){
        fs::File entry = dir.openFile("r");
        String filename = String(entry.name());
        if (!filename.endsWith("edit.htm.gz")){
          if (output != "[") output += ',';
          bool isDir = false;
          output += "{\"type\":\"";
          output += (isDir)?"dir":"file";
          output += "\",\"name\":\"";
          output += filename.substring(1);
          output += "\",\"size\":";
          output += String(entry.size());
          output += "}";
          entry.close();
        }
      }
      output += "]";
      request->send(200, "text/json", output);
      output = String();
    }
    else if(request->hasParam("edit") || request->hasParam("download")){
      request->send(request->_tempFile, request->_tempFile.name(), String(), request->hasParam("download"));
    }
    else {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/edit.htm.gz", "text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
  } else if(request->method() == HTTP_DELETE){
    if(request->hasParam("path", true)){
      SPIFFS.remove(request->getParam("path", true)->value());
      request->send(200, "", "DELETE: "+request->getParam("path", true)->value());
    } else
      request->send(404);
  } else if(request->method() == HTTP_POST){
    if(request->hasParam("data", true, true) && SPIFFS.exists(request->getParam("data", true, true)->value()))
      request->send(200, "", "UPLOADED: "+request->getParam("data", true, true)->value());
    else
      request->send(500);
  } else if(request->method() == HTTP_PUT){
    if(request->hasParam("path", true)){
      String filename = request->getParam("path", true)->value();
      Serial.println(filename);
      if(SPIFFS.exists(filename)){
        request->send(200);
      } else {
        fs::File f = SPIFFS.open(filename, "w");
        if(f){
          f.write((uint8_t)0x00);
          f.close();
          request->send(200, "", "CREATE: "+filename);
        } else {
          request->send(500);
        }
      }
    } else
      request->send(400);
  }
}

void FSEditor::handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
  if(!index){
    if(!_username.length() || request->authenticate(_username.c_str(),_password.c_str())){
      _authenticated = true;
      request->_tempFile = SPIFFS.open(filename, "w");
      _startTime = millis();
    }
  }
  if(_authenticated && request->_tempFile){
    if(len){
      request->_tempFile.write(data,len);
    }
    if(final){
      request->_tempFile.close();
    }
  }
}
