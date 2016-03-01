#include "helpers.h"

bool IsPrefixOfString(std::string &Line, std::string Prefix)
{
    bool Result = false;

    if(Line.substr(0, Prefix.size()) == Prefix)
    {
        Line = Line.substr(Prefix.size()+1);
        Result = true;
    }

    return Result;
}

std::string CreateStringFromTokens(std::vector<std::string> Tokens, int StartIndex)
{
    std::string Text = "";
    for(std::size_t TokenIndex = StartIndex; TokenIndex < Tokens.size(); ++TokenIndex)
    {
        Text += Tokens[TokenIndex];
        if(TokenIndex < Tokens.size() - 1)
            Text += " ";
    }

    return Text;
}

std::vector<std::string> SplitString(std::string Line, char Delim)
{
    std::vector<std::string> Elements;
    std::stringstream Stream(Line);
    std::string Temp;

    while(std::getline(Stream, Temp, Delim))
        Elements.push_back(Temp);

    return Elements;
}

int ConvertStringToInt(std::string Integer)
{
    int IntResult;
    std::stringstream Stream(Integer);
    Stream >> IntResult;
    return IntResult;
}

unsigned int ConvertHexStringToInt(std::string HexString)
{
    unsigned int HexResult = 0xffffff;
    std::stringstream Stream;
    Stream << std::hex << HexString;
    Stream >> HexResult;
    return HexResult;
}

double ConvertStringToDouble(std::string Double)
{
    double DoubleResult;
    std::stringstream Stream(Double);
    Stream >> DoubleResult;
    return DoubleResult;
}

void CreateColorFormat(color *Color)
{
    Color->Format = "r:" + std::to_string(Color->Red) + \
                    " g:" + std::to_string(Color->Green) + \
                    " b:" + std::to_string(Color->Blue) + \
                    " a:" + std::to_string(Color->Alpha);
}

color ConvertHexRGBAToColor(unsigned int Color)
{
    color Result = {};

    Result.Red = ((Color >> 16) & 0xff) / 255.0;
    Result.Green = ((Color >> 8) & 0xff) / 255.0;
    Result.Blue = ((Color >> 0) & 0xff) / 255.0;
    Result.Alpha = ((Color >> 24) & 0xff) / 255.0;

    return Result;
}

std::string GetUTF8String(CFStringRef Temp)
{
    std::string Result;

    if(!CFStringGetCStringPtr(Temp, kCFStringEncodingUTF8))
    {
        CFIndex Length = CFStringGetLength(Temp);
        CFIndex Bytes = 4 * Length + 1;
        char *TempUTF8StringPtr = (char*) malloc(Bytes);

        CFStringGetCString(Temp, TempUTF8StringPtr, Bytes, kCFStringEncodingUTF8);
        if(TempUTF8StringPtr)
        {
            Result = TempUTF8StringPtr;
            free(TempUTF8StringPtr);
        }
    }

    return Result;
}

