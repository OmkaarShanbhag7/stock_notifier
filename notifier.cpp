#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>


using json = nlohmann::json;


size_t WriteCallBack(void* contents , size_t size , size_t nmemb , std::string* userp){
	size_t totalSize = size * nmemb;
	
	userp->append((char*)contents , totalSize);
	return totalSize;
}


int main(){
	CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(result != CURLE_OK)
		return (int) result;

	CURL *curl = curl_easy_init();

	if(curl){
		std::string readBuffer;

		curl_easy_setopt(curl , CURLOPT_URL , "https://query1.finance.yahoo.com/v8/finance/chart/ITCHOTELS.NS");
	
		curl_easy_setopt(curl , CURLOPT_USERAGENT , "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

		curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , WriteCallBack);

		curl_easy_setopt(curl , CURLOPT_WRITEDATA , &readBuffer);

		result = curl_easy_perform(curl);
		
		json jsonData = json::parse(readBuffer);
		
		double regularMarketPrice = jsonData["chart"]["result"][0]["meta"]["regularMarketPrice"];
		double previousClose = jsonData["chart"]["result"][0]["meta"]["previousClose"];
		
		double change = (regularMarketPrice - previousClose) / previousClose * 100;
		if(std::abs(change) >= 2.0){
			std::ostringstream oss;
			oss<<std::fixed<<std::setprecision(2)<<change;
			std::string str = oss.str();

			//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
			//curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
			std::string ntfyBuffer;
			curl_easy_setopt(curl , CURLOPT_WRITEDATA , &ntfyBuffer);

			curl_easy_setopt(curl , CURLOPT_URL , "https://ntfy.sh/StockNotifier");
			
			std::string message = "ITC_HOTELS.NS moved " + str + " % today! ";

			curl_easy_setopt(curl , CURLOPT_POSTFIELDS , message.c_str());

			result = curl_easy_perform(curl);

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);

			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		}

		if(result != CURLE_OK)
		{
			fprintf(stderr,"Request Failed: %s\n",
					curl_easy_strerror(result));
		}

		curl_easy_cleanup(curl);

	}

	curl_global_cleanup();

	return 0;
}

