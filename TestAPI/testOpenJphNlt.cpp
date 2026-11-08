// Regression coverage for OpenJPH's 8-bit NLT lookup-table marker parser.
#include "ojph_file.h"
#include "ojph_message.h"
#include "ojph_params.h"
#include "codestream/ojph_params_local.h"
#include <cstdio>
#include <stdexcept>
#include <vector>

static std::vector<ojph::ui8> marker() {
    // Lnlt=19, component 0, decoded depth 8, LUT-style NLT, two points,
    // Dmin=0, Dmax=UINT32_MAX, eight bits per point, points {0,255}.
    return {0, 19, 0, 0, 7, 2, 0, 1, 0, 0, 0, 0,
            255, 255, 255, 255, 8, 0, 255};
}

static void check(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

int main() {
    try {
        // Trailing bytes model the next codestream marker. The old parser
        // read the whole allocation size into its two-byte point array.
        auto bytes = marker();
        const auto markerSize = bytes.size();
        bytes.resize(markerSize + 64, 0xA5);
        ojph::mem_infile input;
        input.open(bytes.data(), bytes.size());
        ojph::local::param_nlt nlt;
        check(nlt.read(&input), "Valid NLT marker was rejected");
        check(input.tell() == static_cast<ojph::si64>(markerSize),
              "NLT parser consumed the following marker");

        const auto valid = marker();
        // Expected rejections throw after printing; keep the test log quiet.
        ojph::set_error_stream(nullptr);
        for (size_t length = 0; length < valid.size(); ++length) {
            ojph::mem_infile truncated;
            truncated.open(valid.data(), length);
            ojph::local::param_nlt incomplete;
            bool rejected = false;
            try {
                rejected = !incomplete.read(&truncated);
            } catch (const std::exception &) {
                rejected = true;
            }
            check(rejected, "Truncated NLT marker was accepted");
        }
        puts("OpenJPH NLT marker bounds and truncation checks passed");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
