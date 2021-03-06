#include "stdafx.h"
#include "CppUnitTest.h"
#pragma warning(push)
#pragma warning(disable:996)
#include <boost/multi_array.hpp>
#include <folly/File.h>
#include <folly/FileUtil.h>
#include <chrono>
#include <chrono>
#include <regex>
#include <regex>
#include <string>
#include <tinyxml2.h>
#include <vector>
#pragma warning(pop)

#include "../XmlProcess/mpd_format.h"
#include "../XmlProcess/mpd_parser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono_literals;

namespace Test
{
    struct check_result
    {
        template<typename U>
        check_result& operator=(U&& result) {
            using native_type = typename std::decay<U>::type;
            if constexpr (std::is_same<tinyxml2::XMLError, native_type>::value) {
                assert(result == tinyxml2::XMLError::XML_SUCCESS);
            } else if constexpr (std::is_pointer<native_type>::value) {
                assert(result != nullptr);
            } else if constexpr (std::is_same<bool, native_type>::value) {
                assert(result);
            } else {
                throw std::runtime_error{ "unreachable branch" };
            }
            return *this;
        }
    };

    static thread_local check_result result;

    TEST_CLASS(XmlTest) {
    public:

        TEST_METHOD(BoostMultiArrayRef) {
            constexpr auto dim = 2;
            std::vector<int> vec{ 1,2,3,4,5,6 };
            int r = 2, c = 3;
            boost::multi_array_ref<int, dim> ref{ vec.data(),boost::extents[r][c] };
            boost::array<decltype(ref)::index, dim> index{ 1,2 };
            Assert::IsTrue(ref(index) == 6);
            // TODO: Your test code here
        }

        TEST_METHOD(DurationRegex) {
            std::string duration = "PT0H12M14.000S";
            std::string duration2 = "PT1.500S";
            std::regex duration_pattern{ R"(PT((\d+)H)?((\d+)M)?(\d+\.\d+)S)" };
            std::smatch matches;
            result = std::regex_match(duration, matches, duration_pattern);
            auto h = std::stoi(matches[2].str());
            auto m = std::stoi(matches[4].str());
            auto s = std::stoi(matches[5].str());
            auto secs = 1s*(h * 3600 + 60 * m + s);
            std::smatch matches2;
            result = std::regex_match(duration2, matches2, duration_pattern);
            h = matches2[2].matched ? std::stoi(matches[2].str()) : 0;
            m = matches2[4].matched ? std::stoi(matches2[4].str()) : 0;
            auto s2 = matches2[5].matched ? std::stod(matches2[5].str()) : 0;
            auto secs2 = 1000ms * (h * 3600 + 60 * m + s2);
            Assert::IsTrue(secs == 734s);
            Assert::IsTrue(secs2 == 1.5s);
        }

        TEST_METHOD(MpdParserDuration) {
            auto secs = mpd_parser::parse_duration("PT0H12M14.000S");
            auto secs2 = mpd_parser::parse_duration("PT1.500S");
            Assert::IsTrue(secs == 734s);
            Assert::IsTrue(secs2 == 1.5s);
        }

        TEST_METHOD(XmlParseParallel) {
            std::string xml_string;
            result = folly::readFile("F:/Tile/tears_of_steal/4k-part/tos_srd_4K.mpd", xml_string);
            tinyxml2::XMLDocument document;
            result = document.Parse(xml_string.c_str());
            auto* xml_root = document.RootElement();
            auto t = xml_root->Attribute("mediaPresentationDuration");
            mpd_parser parser{ xml_string };
            Assert::IsTrue(true);
        }

        TEST_METHOD(MpdVideoSetAccess) {
            std::string xml_string;
            result = folly::readFile("F:/Tile/tears_of_steal/4k-part/tos_srd_4K.mpd", xml_string);
            mpd_parser parser{ xml_string };
            auto& video_set = parser.video_set(2, 1);
            Assert::IsTrue(video_set.x == 2);
            Assert::IsTrue(video_set.y == 1);
        }
    };
}