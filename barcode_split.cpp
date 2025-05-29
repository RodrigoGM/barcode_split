#include <iostream>
#include <fstream>
#include <string>
#include <zlib.h>
#include <getopt.h>
#include <cstring>   // for strdup()
#include <cstdlib>   // for free()
#include <libgen.h>  // for basename()

const int BUFFER_SIZE = 8192;

// Print help message
void print_usage(const std::string &prog_name) {
    std::cerr << "Usage: " << prog_name
              << " -p <barcode> [-m <mismatches>] [-l <search_len>] [-t] [-o <output_stem>] R1.fastq.gz R2.fastq.gz\n\n"
              << "Required arguments:\n"
              << "  -p <barcode>           Barcode sequence (must be ≥11 nt)\n\n"
              << "Optional arguments:\n"
              << "  -m <int>               Maximum allowed mismatches (default: 1)\n"
              << "  -l <int>               Length of sequence to search for barcode (default: 80)\n"
              << "  -t                     Trim barcode from read sequence and quality\n"
              << "  -o <output_stem>       Output file prefix (default: basename of R1)\n"
              << "  -h                     Show this help message\n\n"
              << "Positional arguments:\n"
              << "  R1.fastq.gz R2.fastq.gz   Paired gzipped FASTQ input files\n";
}

int hammingDistance(const std::string &s1, const std::string &s2) {
    if (s1.length() != s2.length()) return -1;
    int dist = 0;
    for (size_t i = 0; i < s1.length(); ++i)
        if (s1[i] != s2[i]) ++dist;
    return dist;
}

bool readFastq(gzFile file, std::string &l1, std::string &l2, std::string &l3, std::string &l4) {
    char buffer[BUFFER_SIZE];
    if (!gzgets(file, buffer, BUFFER_SIZE)) return false;
    l1 = buffer;
    if (!gzgets(file, buffer, BUFFER_SIZE)) return false;
    l2 = buffer;
    if (!gzgets(file, buffer, BUFFER_SIZE)) return false;
    l3 = buffer;
    if (!gzgets(file, buffer, BUFFER_SIZE)) return false;
    l4 = buffer;

    l1.erase(l1.find_last_not_of("\r\n") + 1);
    l2.erase(l2.find_last_not_of("\r\n") + 1);
    l3.erase(l3.find_last_not_of("\r\n") + 1);
    l4.erase(l4.find_last_not_of("\r\n") + 1);
    return true;
}

bool findBarcode(const std::string &seq, const std::string &barcode, int max_mismatches, int max_len, int &pos_out) {
    size_t max_pos = std::min((size_t)max_len, seq.length());
    size_t len = barcode.length();
    for (size_t i = 0; i + len <= max_pos; ++i) {
        std::string sub = seq.substr(i, len);
        if (hammingDistance(sub, barcode) <= max_mismatches) {
            pos_out = static_cast<int>(i);
            return true;
        }
    }
    return false;
}

void writeFastqGz(gzFile out, const std::string &l1, const std::string &l2,
                  const std::string &l3, const std::string &l4) {
    gzprintf(out, "%s\n%s\n%s\n%s\n", l1.c_str(), l2.c_str(), l3.c_str(), l4.c_str());
}

std::string basename_noext(const std::string &path) {
    char *cpath = strdup(path.c_str());
    std::string name = basename(cpath);
    free(cpath);
    size_t dot = name.find_first_of('.');
    return (dot == std::string::npos) ? name : name.substr(0, dot);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        print_usage(argv[0]);
        return 1;
    }

    std::string barcode;
    int mismatches = 1;
    int search_len = 80;
    bool trim = false;
    std::string out_stem;

    int opt;
    while ((opt = getopt(argc, argv, "m:l:tp:o:h")) != -1) {
        switch (opt) {
            case 'm': mismatches = std::atoi(optarg); break;
            case 'l': search_len = std::atoi(optarg); break;
            case 't': trim = true; break;
            case 'p': barcode = optarg; break;
            case 'o': out_stem = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default:  print_usage(argv[0]); return 1;
        }
    }

    if (optind + 2 != argc || barcode.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    std::string r1_path = argv[optind];
    std::string r2_path = argv[optind + 1];
    if (out_stem.empty()) out_stem = basename_noext(r1_path);

    std::string out_r1_name = out_stem + "_R1.fastq.gz";
    std::string out_r2_name = out_stem + "_R2.fastq.gz";

    if (barcode.length() < 11) {
        std::cerr << "Error: Barcode must be at least 11 nucleotides.\n";
        return 1;
    }

    gzFile r1_file = gzopen(r1_path.c_str(), "rb");
    gzFile r2_file = gzopen(r2_path.c_str(), "rb");
    gzFile out_r1 = gzopen(out_r1_name.c_str(), "wb");
    gzFile out_r2 = gzopen(out_r2_name.c_str(), "wb");

    if (!r1_file || !r2_file || !out_r1 || !out_r2) {
        std::cerr << "Error opening input or output files.\n";
        return 1;
    }

    std::string l1_R1, l2_R1, l3_R1, l4_R1;
    std::string l1_R2, l2_R2, l3_R2, l4_R2;
    size_t matched_count = 0;

    while (readFastq(r1_file, l1_R1, l2_R1, l3_R1, l4_R1) &&
           readFastq(r2_file, l1_R2, l2_R2, l3_R2, l4_R2)) {

        int pos1 = -1, pos2 = -1;
        bool found1 = findBarcode(l2_R1, barcode, mismatches, search_len, pos1);
        bool found2 = findBarcode(l2_R2, barcode, mismatches, search_len, pos2);

        if (found1 && found2) {
            std::string s2_R1 = l2_R1, s2_R2 = l2_R2;
            std::string q2_R1 = l4_R1, q2_R2 = l4_R2;

            if (trim) {
                s2_R1.erase(pos1, barcode.length());
                q2_R1.erase(pos1, barcode.length());
                s2_R2.erase(pos2, barcode.length());
                q2_R2.erase(pos2, barcode.length());
            }

            writeFastqGz(out_r1, l1_R1, s2_R1, l3_R1, q2_R1);
            writeFastqGz(out_r2, l1_R2, s2_R2, l3_R2, q2_R2);
            ++matched_count;
        }
    }

    gzclose(r1_file);
    gzclose(r2_file);
    gzclose(out_r1);
    gzclose(out_r2);

    std::cout << "Matched read pairs: " << matched_count << "\n";
    std::cout << "Output written to:\n  " << out_r1_name << "\n  " << out_r2_name << "\n";
    return 0;
}
