#include <iostream>
#include <hanzi-to-pinyin/hanzi_to_pinyin.h>

int main()
{
    try
    {
        HanZiToPinYin_NS::HanZiToPinYin hanzi_to_pinyin;

        hanzi_to_pinyin.InitDefault();

        std::string input(u8"你好");

        std::string pinyin_with_tone;
        std::string pinyin_without_tone;
        std::string pinyin_first_letters;

        if (!hanzi_to_pinyin.GetPinYin(input, &pinyin_with_tone, &pinyin_without_tone, &pinyin_first_letters))
        {
            std::cerr << "HanZiToPinYin::GetPinYin failed\n";
            return -2;
        }

        std::cout << "---Input---\n"
                  << input
                  << "\n---Output---"
                  << "\nwith tone: " << pinyin_with_tone
                  << "\nwithout tone: " << pinyin_without_tone
                  << "\nfirst letters: " << pinyin_first_letters
                  << "\n---End---"
                  << std::endl;
    }
    catch (std::exception & e)
    {
        std::cerr << e.what() << std::endl;

        return -1;
    }

    return 0;
}
