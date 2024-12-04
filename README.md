Simple search engine utilizing [TF-IDF](https://en.wikipedia.org/wiki/Tf%E2%80%93idf).

## Building

```console
$ make build
```

This spits out an executable called `tfidf`.

## Usage

```console
Usage: tfidf -s [FILE] [OPTIONS]

Arguments:
    -s [FILE]     Input file in skvs format
    -n [AMOUNT]   How many search results to show
    -q [QUERY]    Run a single query

Options:
    -i            Interactive mode
```

## SKVS

`tfidf` uses a custom data format (called `skvs`) for loading corpuses. It consist of pairs where the layout looks like this:

| What? | 1st element length (X )| 1st element | 2nd element length (Y) | 2nd element |
---------------------------------------------------------------------
| Size (bytes) | 4 | X | 4 | Y |

All lengths are stored in network order.

Here is an example of a python script to generate a curpus from a CSV file.

```python
import csv
import struct

with open("corpus.skvs", "wb") as kvs:
    with open("some_dataset.cvs") as csvf:
        reader = csv.DictReader(csvf)
        for row in reader:
            titlebytes = bytes(row["title"], encoding="utf-8")
            contentsbytes = bytes(row["contents"], encoding="utf-8")

            kvs.write(struct.pack("!I", len(titlebytes)))
            kvs.write(titlebytes)

            kvs.write(struct.pack("!I", len(contentsbytes)))
            kvs.write(contentsbytes)
```

NOTE: All the stemming and stop word removal should be done when creating the corpus, as `tfidf` does not do this for you.
