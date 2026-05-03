#include "api_comm.h"

int CreateRoute(const GPSData& currentLocation, const GPSData& destination) {
    HTTPClient http;
    String routeUrl = String(ROOT_PATH) + String(ROUTE_ENDPOINT);
    routeUrl += "?slat=" + String(currentLocation.lat, 6);
    routeUrl += "&slon=" + String(currentLocation.lng, 6);
    routeUrl += "&tlat=" + String(destination.lat, 6);
    routeUrl += "&tlon=" + String(destination.lng, 6);
    routeUrl += "&device_id=demo";

    http.begin(routeUrl);
    Serial.printf("[API] Creating route: %s\n", routeUrl.c_str());
    int httpResponseCode = http.POST("");
    Serial.printf("[API] CreateRoute response code: %d\n", httpResponseCode);

    http.end();
    return httpResponseCode;
}

String GetNavigationInstruction(const GPSData& currentLocation) {
    HTTPClient http;
    String url = String(ROOT_PATH) + String(INSTRUCT_ENDPOINT);
    url += "?current_lat=" + String(currentLocation.lat, 6);
    url += "&current_lon=" + String(currentLocation.lng, 6);
    url += "&device_id=demo";

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
        // Có chỉ dẫn âm thanh
        Serial.printf("[API] Navigation instruction: %d (New audio available)\n", httpResponseCode);
        http.end();
        return url; 
    } 
    else if (httpResponseCode == 201) {
        // Trạng thái COMPLETE: Đã đến đích
        Serial.println("[API] Navigation COMPLETE! Destination reached.");
        http.end();
        return "complete";
    }
    else if (httpResponseCode == 204) {
        // Im lặng (No Content)
        // Serial.println("[API] Navigation: 204 (No instruction yet)");
        http.end();
        return "";
    } else {
        Serial.printf("[API] Navigation error code: %d\n", httpResponseCode);
    }

    http.end();
    return ""; 
}