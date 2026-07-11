#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <vector>


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
		std::vector<std::string> watchList = {
			"HINDCOPPER.NS" , "ITCHOTELS.NS" , "ABB.NS" , "COROMANDEL.NS" , "EVERESTIND.NS" , "ABFRL.NS" ,"TATASTEEL.NS" ,  "VBL.NS" , "SANOFICONR.NS" , "INDRAMEDCO.NS" , "ASIANPAINT.NS" , "EKC.NS" , "BIRLACORPN.NS" , "TATAMOTORS.NS" , "BEL.NS" , "ADANIGREEN.NS" , "NAVKARCORP.NS" , "RKFORGE.NS" , "GLENMARK.NS" , "EIHOTEL.NS" , "NTPC.NS" , "BSE.NS" , "TATAPOWER.NS" , "SANOFI.NS" , "TVSHLTD.NS" , "NHPC.NS" , "ALEMBICLTD.NS" , "ZYDUSLIFE.NS" , "IOC.NS" , "IEX.NS" , "GAIL.NS" , "STARCEMENT.NS" , "FORTIS.NS" , "FINPIPE.NS" , "RECLTD.NS" ,"RELIANCE.NS" ,  "ACC.NS" , "SUNPHARMA.NS" , "ABCAPITAL.NS" ,  "TVSSCS.NS" , "ITC.NS" , "KPIL.NS" , "JSWINFRA.NS" , "SPARC.NS"
		};

		for(const std::string& ticker : watchList){
			std::string readBuffer;
			std::string api_url = "https://query1.finance.yahoo.com/v8/finance/chart/" + ticker;
			curl_easy_setopt(curl , CURLOPT_URL , api_url.c_str());

			curl_easy_setopt(curl , CURLOPT_USERAGENT , "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

			curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , WriteCallBack);

			curl_easy_setopt(curl , CURLOPT_WRITEDATA , &readBuffer);

			result = curl_easy_perform(curl);

			json jsonData = json::parse(readBuffer);
			
			if(jsonData["chart"]["result"].is_null() || jsonData["chart"]["result"].empty())
				continue;
	
			auto& meta = jsonData["chart"]["result"][0]["meta"];
			if(meta["regularMarketPrice"].is_null() || meta["previousClose"].is_null()){
				continue;
			}

			double regularMarketPrice = jsonData["chart"]["result"][0]["meta"]["regularMarketPrice"];
			double previousClose = jsonData["chart"]["result"][0]["meta"]["previousClose"];

			double change = (regularMarketPrice - previousClose) / previousClose * 100;
			if(std::abs(change) >= 2.0){
				std::ostringstream oss;
				oss<<std::fixed<<std::setprecision(2)<<change;
				std::string str = oss.str();
				
				std::string ntfyBuffer;

				curl_easy_setopt(curl , CURLOPT_WRITEDATA , &ntfyBuffer);

				curl_easy_setopt(curl , CURLOPT_URL , "https://ntfy.sh/StockNotifier");

				std::string message = ticker + " moved " + str + " % today! ";

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
		}
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return 0;
}

