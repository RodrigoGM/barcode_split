# barcode_split

## Disclosures
This tool was created with support from ChatGPT.  Verified using real sequence data in MacOS and Linux CentOS.

## Description

This tool is intended to be used in sequencing libraries with in-line barcodes i.e. the
barcode is part of the R1/R2 outputs.  Our library prep method is currently designed to
insert both R1 and R2 require the same barcode.



## Usage

| Flag       | Description                        | Default             |
| ---------- | ---------------------------------- | ------------------- |
| `-p`       | Barcode pattern (required)         | —                   |
| `-m`       | Max mismatches                     | 1                   |
| `-l`       | Barcode search window (in bp)      | 80                  |
| `-t`       | Trim barcode from sequence/quality | off                 |
| `-o`       | Output stem                        | basename of R1 file |
| `-h`       | Print help                         | -                   |
| Final args | Input R1/R2 FASTQ (gzipped)        | —                   |

``` bash
./barcode_split -p ACGTGGTCAGA -m 2 -l 60 -t -o matched_reads sample_R1.fastq.gz sample_R2.fastq.gz

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
brew install gcc zlib

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

