# `readspss` -- importing SPSS files to R

Welcome to the `readspss` package. The package is written from scratch with
additional code for special cases. Starting as a side project for to learn Rcpp
the package did grow over the years. Today `readspss` is well tested using
every sav, por and zsav file I could lay my hands on. Import is considered 
feature complete, write support is available, just not yet for every feature.

## Installation

Installation is provided using `drat` or `devtools`. The `drat` repo provides
binary files for Windows in case one has no rtools installed.

With `drat`:
```R
drat::addRepo("JanMarvin")
install.packages("readspss")
```

With `devtools`:
```R
devtools::install_git("https://github.com/JanMarvin/readspss.git")
```

## Import

For the import of sav and por files `read.sav()` and `read.por()` are available.

```{R}
library(readspss)

# example using sav
fl_sav <- system.file("extdata", "electric.sav", package = "readspss")
ds <- read.sav(fl_sav)

# example using por
fl_por <- system.file("extdata", "electric.sav", package = "readspss")
dp <- read.sav(fl_por)
```

Both functions return data.frame objects, containing numerics, dates, factors or
characters. 

### Import options

For user specific demand the package supports many option available in `foreign`
such as `convert.factors` and `use.missings`. If one is familiar with said
package, one should have no problem adapting to `readspss`. If for example one
does not want to have factors since they work differently in R than in SPSS this
can be achieved using the following code.

```{R}
# example using sav
fl_sav <- system.file("extdata", "electric.sav", package = "readspss")
ds <- read.sav(fl_sav, convert.factors = FALSE)

# example using por
fl_por <- system.file("extdata", "electric.sav", package = "readspss")
dp <- read.sav(fl_por, convert.factors = FALSE)
```

Since many features are self explanatory not all will be explained. Of course 
`readspss` can handle sav files in different encodings; it handles file sets 
without data; all types of missings SPSS developers over the years came up with;
short, long and longer strings; little and big endian files; sav, por and zsav
compressed files; files without valid header information; old and new SPSS
files. As stated above, every SPSS file I came across and during the development
I came across many.

Using code by Ben Pfaff `readspss` can handle encrypted SPSS files.


```{R}
flu <- system.file("extdata", "hotel.sav", package="readspss")
fle <- system.file("extdata", "hotel-encrypted.sav", package="readspss")

df_u <- read.sav(flu)
df_e <- read.sav(fle, pass = "pspp")
```

### Import details

If available in the SPSS file, the resulting data.frame will contain attributes
such as the datalabel, timestamp, filelabel, additional documentation and 
information regarding missings, labels, file encoding. A complete list of
attributes can be found using `?read.sav` and `?read.por`.

## Export

R data.frame objects can be exported using `write.sav()` and `write.por()`.

```{R}
libray(readspss)

write.sav(cars, filepath = "cars.sav")

write.por(cars, filepath = "cars.por")
```

Export provides a few options to add a label, for compression of sav files and
conversion of dates. Currently it is not possible to export zsav-files and it is
not possible to export strings longer than 255 chars.


## Package development

One may wonder, why does the world need another package to import SPSS data to 
the R world. Similar tasks can be done by `foreign`, `memisc` and `haven` 
package.
Well the first two packages use code from older releases of PSPP and R-Core most
likely has neither time nor a need to update their codebase to a newer PSPP 
release. Still over the years the SPSS file format has changed. Not drastically 
but new features such as long strings were implemented. Features that `foreign` 
cannot handle. The newest of the three aforementioned packages, `haven`, is a 
wrapper around the `ReadStat` C library. The package development began around
the time we started with [`readstata13`](https://github.com/sjewo/readstata13)
so it is around quite some time now. Contrary to many other people in the R 
world, I am not a huge fan of `tibbles` which are an integral part of `haven`.
One can agree that this is a minor problem. My bigger problem with the package 
is, that I am not yet convinced that `ReadStat` and `haven` are tested enough. 
Even though I am sure that authors of both made sure that in most cases their 
package works, in many cases it does not. During the development process of 
`readspss` I reported a few bugs to the haven package. Among them were 
incorrectly trimmed long strings and a severe bug where por-files read simply 
incorrect values. All errors were found using publicly available data files, 
writing unit tests and comparing data across different R-packages, PSPP and 
various versions of SPSS. Until I see that such behavior is adopted by other
packages, I simply do not trust them. If the import process of data fails, one
does not have to worry about everything that follows.

The development of `readspss` began once development of `readstata13` slowed
down. Having written most of the `c++` code to import dta-files, I learned a lot
about binary files and Rcpp development. Since SPSS was another statistical
software used at the university I worked at that time, it felt natural to have
a look at sav-files. Shortly after I learned that the dta-file documentation is
priceless, not available for SPSS and development ceased for quite some time.
In February 2018 I changed jobs, resulting in many train rides. A project was
needed and development began again. Using the PSPP documentation and countless 
hours of trial and error lead to the current state of the package.


## Thanks

`readspss` uses code of Ben Pfaff for the encryption part. It uses code from TDA
by Goetz Rohwer and Ulrich Poetter for the conversion of numerics in the por-
parser. The PSPP documentation was a huge help. Without the testing by Ulrich
Poetter this package would not be as complete as it is.

## Last words

Even though `readspss` is tested a lot, it would be great, if users would
test their imports and exports and report their experience. Open an issue on the
github page or write me an email and let me know what you think. Bug free code 
does not exist.