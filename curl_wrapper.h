#ifndef CURL_WRAPPER_H_INCLUDED
#define CURL_WRAPPER_H_INCLUDED

//curl stuff for getting info from the chrome cast
struct MemoryStruct {
  char *memory;
  size_t size;
};

char* HTTPRequest(const char* url);

#endif // CURL_WRAPPER_H_INCLUDED
