/*
 * Copyright (C) 2018 Jan Marvin Garbuszus
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <fstream>
#include <streambuf>
#include <regex>

#include "spss.h"

Rcpp::List read_sav_known_n (std::istream& sav,
               bool swapit, bool cflag, bool debug,
               int32_t n, int32_t kv,
               Rcpp::IntegerVector vtyp,
               Rcpp::NumericVector res,
               std::vector<int> vartype) {

  // 1. Create Rcpp::List
  Rcpp::List df(kv);
  for (uint16_t i=0; i<kv; ++i)
  {
    int const type = vtyp[i];
    switch(type)
    {
    case 0:
      SET_VECTOR_ELT(df, i, Rcpp::NumericVector(Rcpp::no_init(n)));
      break;

    default:
      SET_VECTOR_ELT(df, i, Rcpp::CharacterVector(Rcpp::no_init(n)));
    break;
    }
  }

  int32_t unk8=0;
  unk8 = readbin(unk8, sav, swapit); // 0


  bool eof = 0;
  uint8_t val_b = 0;
  int32_t nn = 0, kk = 0;

  // data is read in 8 byte chunks. k*n/8 (data remains)
  double chunk = 0, val_d = 0;


  if (debug) {
    Rprintf("cflag: %d\n", cflag);
  }


  // cflag 1 = compression int8_t - bias
  if (cflag) {

    std::string start = "";
    int32_t res_i = 0, res_kk = 0, kk_i = 0;

    while (!eof) { // start data import until nn = n

      Rcpp::checkUserInterrupt();


      // data is stored rowwise-ish.

      // chunk is 8 bit long. it gives the structure of the data. If it contains
      // only uint8_t it stores 8 vals. If data contains doubles it stores a
      // 253 and the next 8 byte will be the double.

      chunk = readbin(val_d, sav, 0);

      Rcpp::IntegerVector chunkvec(8);

      // therefor with respect to the required data structure (numerics and
      // strings) the data has to be read.
      // e.g. if there are 2 vals, in the first 8 bit may be 4 rows.

      union {
        double d;
        uint8_t byte[8];
      } u;

      u.d = chunk;

      for (int8_t i=0; i<8; ++i)
      {

        val_b = u.byte[i];

        // 0 = empty
        // 1:251 = numeric/string
        // each 253 follow up on a string or double in next block

        int32_t len = 0;
        int32_t const type = vartype[kk_i];
        len = type;

        // kk_i is index of the original number of variables
        // kk_i is reset once kv the new number of varialbes is reachead
        ++kk_i;


        // if (debug) {
        //   Rprintf("val_b: %d - type: %d - kk: %d - nn: %d\n",
        //           val_b, type, kk+1, nn+1);
        //
        //   Rprintf("res_i: %d\n", res_i);
        // }


        // res_kk is the amount of chunks required to read until the
        // string is completely read
        res_kk = res[kk];

        switch (val_b)
        {

        case 0:
        {
          --kk;
          break;
          // ignored
        }

        default: // (val_b >= 1 & val_b <= 251) {
        {

          switch(type)
        {

        case 0:
        {
          REAL(VECTOR_ELT(df,kk))[nn] = val_b - 100;
          break;
        }

        default:
        {

          if (len==-1 || (len !=0 && len !=8) )
            len = 8;

          // beginning of a new string
          std::string val_s (len, '\0');
          val_s = readstring(val_s, sav, val_s.size());
          start.append( val_s );

          // if res_i == res_kk the full string was read and
          // can be written else continue the string
          if (res_i == res_kk-1) {

            // trim additional whitespaces to the right
            start = std::regex_replace(start,
                                       std::regex(" +$"), "$1");

            Rcpp::as<Rcpp::CharacterVector>(df[kk])[nn] = start;

            // string completly written, reset start and res_i
            // and switch to next cell
            start = "";
            res_i = 0;
          } else {
            // string will be continued
            ++res_i;
          }

          break;
        }

        }

          break;
        }

        case 252:
        {
          // 252 should be end of file, but as many things
          // it is not required to be inside the file
          eof = true;
          break;
        }

        case  253:
        {
          //           Rcpp::Rcout << "## Debug ... 253" << std::endl;
          //           Rprintf("nn %d & kk %d \n", nn, kk);
          switch(type)
        {

        case 0:
        {
          val_d = readbin(val_d, sav, swapit);
          REAL(VECTOR_ELT(df,kk))[nn] = val_d;
          // Rprintf("%f \n", val_d);
          break;
        }

        default:
        {

          // spss length 1:251 indicate a string. the value is the string
          // size. obvious spss uses the size to determine the size of the
          // string. there are two possible problems.
          // 1. len can be 1:7 in this case we know the max string size of the
          // variable is less than 8 bit long. still the field to read is 8 bit
          // long.
          // 2. the string is spread across different internal strings. in this
          // case we know the max size, still have to read each 8bit field.
          // maybe the max size can be used to have a second opinion wheather
          // or not a field contains a numeric or character. Following fields
          // have len -1.

          if (len==-1 || (len !=0 && len !=8) )
            len = 8;

          std::string val_s (len, '\0');
          val_s = readstring(val_s, sav, val_s.size());
          start.append( val_s );


          if (res_i == res_kk-1) {

            // trim additional whitespaces to the right
            start = std::regex_replace(start,
                                       std::regex(" +$"), "$1");

            Rcpp::as<Rcpp::CharacterVector>(df[kk])[nn] = start;

            // reset
            start = "";
            res_i = 0;
          } else {
            ++res_i;
          }


          break;
        }

        }

          break;
        }

        case 254:
        {
          // 254 indicates that string chunks read before should be
          // interpreted as a single string.

          if (res_i == res_kk-1) {

          // trim additional whitespaces to the right
          start = std::regex_replace(start,
                                     std::regex(" +$"), "$1");

          Rcpp::as<Rcpp::CharacterVector>(df[kk])[nn] = start;

          // reset start
          start = "";
          res_i = 0;
        } else {
          start.append("        ");
          ++res_i;
        }

        break;
        }

        case 255:
        {
          // 255 is a missing value in spss files.
          //
          switch(type)
        {

        case 0:
        {
          // Rcout << NA_REAL << std::endl;
          REAL(VECTOR_ELT(df,kk))[nn] = NA_REAL;
          break;
        }
        default:
        {
          Rcpp::as<Rcpp::CharacterVector>(df[kk])[nn] = NA_STRING;
          break;
        }
          break;
        }

        }
        }



        // variable is read
        if (res_i == 0)
          ++kk;

        // Update kk iterator. If kk is k, update nn to start in next row.
        if (kk == kv) {
          ++nn;

          // Rprintf("nn: %d", nn);
          // some files are not ended with 252, ensure that no out of bounds
          // error occures.
          if (nn == n) {
            eof = true;

            if (debug)
              Rcpp::Rcout << "stop: eof" << std::endl;

            break;
          }

          // always check if eof is reached
          eof = sav.eof();

          // reset k and res_kk
          kk = 0;
          kk_i = 0;
        }

      }

    }

  } else {

    kk = 0;

    std::string val_s = "";

    for (int ii = 0; ii < n*kv; ++ii) {

      int32_t const type = vtyp[kk];

      switch(type)
      {

      case 0:
      {
        val_d = NA_REAL;
        val_d = readbin(val_d, sav, swapit);
        REAL(VECTOR_ELT(df,kk))[nn] = val_d;
        break;
      }

      default:
      {

        double len = type;

        len = ceil(len/8) * 8;

        std::string val_s ((int32_t)len, '\0');
        val_s = readstring(val_s, sav, val_s.size());

        // shorten the string to the actual size reported by SPSS
        val_s.erase(type, std::string::npos);

        // trim additional whitespaces
        val_s = std::regex_replace(val_s,
                                   std::regex("^ +| +$"), "$1");

        // Rcpp::Rcout << val_s << std::endl;
        Rcpp::as<Rcpp::CharacterVector>(df[kk])[nn] = val_s;

        break;
      }

      }

      ++kk;

      if (kk == kv) {
        ++nn;
        kk = 0;
      }

    }
  }

  return(df);
}