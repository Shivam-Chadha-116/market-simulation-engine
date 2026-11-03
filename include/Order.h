#pragma once/*If this header gets included multiple times during compilation, process its contents only once.+ .h extension is for header fi;es*/
#include<cstdint> /*this here so we can use uint64_t*/
enum class Side{/*enum class is here because it basically predefined b=variables name for us so that we dont accidently type it wrong ,tbh its not the correct definition but it works for me*/
    BUY,SELL
};

struct Order{
    uint64_t timestamp; /*unint64_t means u for unsigned int for integer 64 means 64 bit ,its better to use than normal int as normal int is only 32 bit*/
    uint64_t orderId;
    uint64_t price; /*we have to change this later due to rounding off*/
    uint64_t quantity;
    Side side;
};
 