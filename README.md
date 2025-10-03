# barcode_split


## Description

This tool is intended to be used in sequencing libraries with in-line barcodes i.e. the
barcode is part of the R1/R2 outputs.  Our library prep method is currently designed to
insert both R1 and R2 require the same barcode.

`barcode_split` now supports two modes:
1. **Standard mode**: Searches for barcode sequences with allowed mismatches in `-l` search window
2. **UA-anchored mode**: Searches for barcode immediately upstream of Universal Adapter (UA) sequence for higher specificity

## Usage

### Argument Description

| Flag       | Description                           | Default             |
| ---------- | ------------------------------------- | ------------------- |
| `-p`       | Barcode pattern (required)            | —                   |
| `-m`       | Max mismatches in barcode             | 1                   |
| `-u`       | Universal Adapter sequence            | —                   |
| `-M`       | Max mismatches in UA sequence         | 2                   |
| `-l`       | Barcode search window (in bp)         | 80                  |
| `-t`       | Trim barcode from sequence/quality    | off                 |
| `--no-rc`  | Disable reverse complement search     | off                 |
| `-o`       | Output stem                           | basename of R1 file |
| `-h`       | Print help                            | -                   |
| Final args | Input R1/R2 FASTQ (gzipped)           | —                   |


### Examples

**Standard mode (original behavior):**

``` bash
## search for ACGTGGTCAGA with two mismatches in the first 60 bp of the reads, trim the barcode,
##  and send output to 'matched_reads_R{1,2}.fastq.gz
./barcode_split -p ACGTGGTCAGA -m 2 -l 60 -t -o matched_reads sample_R1.fastq.gz sample_R2.fastq.gz

```
**UA-anchored mode:**

``` bash
## Search for GGCCAGTATGG with up to 1 mismatch, then check if it's followed by  TGTGTTGGGTGTGTTTGG with ≤ 2
## mismatches (UA-anchor), within the first 80 bp (Default), trim the sequences and 
## send output to `matched_reads_R{1,2}.fastq.gz
./barcode_split -p GGCCAGTATGG -u TGTGTTGGGTGTGTTTGG -m 1 -M 2 -t -o matched_reads sample_R1.fastq.gz sample_R2.fastq.gz

```


## Installation
### Dependencies 

| Dependency      | Purpose                               | Required Version |
|:----------------|:--------------------------------------|:-----------------|
| `g++` (GNU C++) | Compile the C++ source code           | C++11 or higher  |
| `zlib`          | Reading and writing `.fastq.gz` files | Any version ≥1.2 |

* :penguin: For debian/linux
``` bash
sudo apt install g++ zlib1g-dev
```

* :penguin: For RedHat/CentOS

``` bash
sudo yum install g++ zlib1g-dev

```

* :apple: For MacOS
``` bash
xcode-select --install
brew install gcc zlib
```

# For macOS with clang
``` bash
clang++ -std=c++11 -O2 -stdlib=libc++ -lz -o barcode_split barcode_split.cpp
```

### compilation
``` bash
g++ -std=c++11 -lz -o barcode_split barcode_split.cpp
```


## Test run

``` bash
./barcode_split -p ACGTGGTCAGA -m 0 -o test/matched_set1 test/test_R{1,2}.fastq.gz

./barcode_split -p TTACCGGATTA -m 0 -o test/matched_set2 test/test_R1.fastq.gz test/test_R2.fastq.gz
```

## Modes Explained

### Standard Mode
* Searches for barcode sequence anywhere within the search window
* Allows specified mismatches in barcode sequence
* May be susceptible to barcode crosstalk (false positive matches)

### UA-Anchored Mode (Recommended)
* Requires -u parameter with Universal Adapter sequence
* Searches for barcode immediately upstream of UA sequence
* Independently validates both barcode and UA with separate mismatch tolerances
* Significantly reduces false positive matches from genome sequence
* When trimming (-t), removes both barcode and UA sequences

## Troubleshooting
### Common Issues
* Getting more reads than expected : Likely excess barcode matches in standard mode.
  * Solution : If your design permits it, use UA-anchored mode with -u [seq],
               else, reduce mismatches, and shorten the search space
			   
* Getting fewer reads than expected : Mismatch tolerances may be too strict
  * Solution : Increase -m and/or -M values

* Compilation errors on macOS : Missing Xcode command line tools
  * Solution : Run xcode-select --install

## Performance Considerations
* Use -O2 or -O3 optimization flags during compilation
* UA-anchored mode is slightly slower but much more accurate
* Adjust -l (search window) based on your library design


## Disclosures
This tool was created with support from ChatGPT.  Verified using real sequence data in MacOS and Linux CentOS.
