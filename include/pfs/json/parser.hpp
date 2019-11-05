////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-json](https://github.com/semenovf/pfs-json) library.
//
// Changelog:
//      2019.10.05 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "json.hpp"
// #include <pfs/algo/advance.hpp>
// #include <pfs/algo/compare.hpp>
#include <algorithm>
#include <bitset>
// #include <iterator>
// #include <limits>
// #include <map>
// #include <string>
#include <utility>
// #include <cerrno>
// #include <cstdlib>
// #include <cstdio>

namespace pfs {
namespace json {

/* In short the main difference between RFC 7159 and RFC 4627 in one rule:
 *
 * RFC 4627: JSON-text = object / array
 * RFC 7159: JSON-text = ws value ws
 *
 */

/*
 *  RFC 7159:  The JavaScript Object Notation (JSON) Data Interchange Format
 *  URL: https://tools.ietf.org/html/rfc7159
 *  ----------------------------------------------------------------------------
 *
 *  A JSON text is a sequence of tokens.  The set of tokens includes six
 *  structural characters, strings, numbers, and three literal names.
 *
 *  A JSON text is a serialized object or array.
 *
 *  RFC 4627: JSON-text = object / array
 *  RFC 7159: JSON-text = ws value ws
 *
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  ws = *(
 *      %x20 /              ; Space
 *      %x09 /              ; Horizontal tab
 *      %x0A /              ; Line feed or New line
 *      %x0D                ; Carriage return
 *  )
 *  ----------------------------------------------------------------------------
 *
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  begin-array     = ws %x5B ws  ; [ left square bracket
 *  begin-object    = ws %x7B ws  ; { left curly bracket
 *  end-array       = ws %x5D ws  ; ] right square bracket
 *  end-object      = ws %x7D ws  ; } right curly bracket
 *  name-separator  = ws %x3A ws  ; : colon
 *  value-separator = ws %x2C ws  ; , comma
 *  ----------------------------------------------------------------------------
 *
 *  Values.
 *  ===========================================================================
 *  A JSON value MUST be an object, array, number, or string, or one of
 *  the following three literal names:
 *
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  false null true
 *  ----------------------------------------------------------------------------
 *
 *  The literal names MUST be lowercase.  No other literal names are
 *  allowed.
 *
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  value = false / null / true / object / array / number / string
 *  false = %x66.61.6c.73.65   ; false
 *  null  = %x6e.75.6c.6c      ; null
 *  true  = %x74.72.75.65      ; true
 *  ----------------------------------------------------------------------------
 *
 *  Objects.
 *  ============================================================================
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  object = begin-object [ member *( value-separator member ) ] end-object
 *
 *  member = string name-separator value
 *  ----------------------------------------------------------------------------
 *
 *  Arrays.
 *  ============================================================================
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  array = begin-array [ value *( value-separator value ) ] end-array
 *  ----------------------------------------------------------------------------
 *
 *  Numbers.
 *  ============================================================================
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  number = [ minus ] int [ frac ] [ exp ]
 *  decimal-point = %x2E       ; .
 *  digit1-9 = %x31-39         ; 1-9
 *  e = %x65 / %x45            ; e E
 *  exp = e [ minus / plus ] 1*DIGIT
 *  frac = decimal-point 1*DIGIT
 *  int = zero / ( digit1-9 *DIGIT )
 *  minus = %x2D               ; -
 *  plus = %x2B                ; +
 *  zero = %x30                ; 0
 *  ----------------------------------------------------------------------------
 *
 *  Strings.
 *  ============================================================================
 *  RFC 4627:
 *  RFC 7159:
 *  ----------------------------------------------------------------------------
 *  string = quotation-mark *char quotation-mark
 *
 *  char = unescaped /
 *      escape (
 *          %x22 /          ; "    quotation mark  U+0022
 *          %x5C /          ; \    reverse solidus U+005C
 *          %x2F /          ; /    solidus         U+002F
 *          %x62 /          ; b    backspace       U+0008
 *          %x66 /          ; f    form feed       U+000C
 *          %x6E /          ; n    line feed       U+000A
 *          %x72 /          ; r    carriage return U+000D
 *          %x74 /          ; t    tab             U+0009
 *          %x75 4HEXDIG )  ; uXXXX                U+XXXX
 *
 *  escape = %x5C              ; \
 *  quotation-mark = %x22      ; "
 *  unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
 *  ----------------------------------------------------------------------------
 */

template <typename T>
using range = std::pair<T,T>;

using std::begin;
using std::end;

enum parse_policy_flag {
      allow_object_root_element
    , allow_array_root_element
    , allow_number_root_element
    , allow_string_root_element
    , allow_boolean_root_element
    , allow_null_root_element

    // Clarification:
    // Compare original 'number' grammar with modified:
    // Original:
    //      number = [ minus ] int ...
    // Modified:
    //      number = [ minus / plus] int ...
    , allow_positive_signed_number

    , parse_policy_count
};

using parse_policy_set = std::bitset<parse_policy_count>;

////////////////////////////////////////////////////////////////////////////////
// Policy satisfying RFC 4627
////////////////////////////////////////////////////////////////////////////////
inline parse_policy_set rfc4627_policy ()
{
    parse_policy_set result;
    result.set(allow_object_root_element, true);
    result.set(allow_array_root_element, true);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Policy satisfying RFC 7159
////////////////////////////////////////////////////////////////////////////////
inline parse_policy_set rfc7159_policy ()
{
    parse_policy_set result;
    result.set(allow_object_root_element, true);
    result.set(allow_array_root_element, true);
    result.set(allow_number_root_element, true);
    result.set(allow_string_root_element, true);
    result.set(allow_boolean_root_element, true);
    result.set(allow_null_root_element, true);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Policy satisfying JSON5
////////////////////////////////////////////////////////////////////////////////
inline parse_policy_set json5_policy ()
{
    parse_policy_set result = rfc7159_policy();

    // TODO Add JSON5 specific policies here

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Strict policy
////////////////////////////////////////////////////////////////////////////////
inline parse_policy_set strict_policy ()
{
    return rfc7159_policy();
}

////////////////////////////////////////////////////////////////////////////////
// Relaxed policy
////////////////////////////////////////////////////////////////////////////////
inline parse_policy_set relaxed_policy ()
{
    parse_policy_set result = rfc7159_policy();
    result.set(allow_positive_signed_number, true);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Number context
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator>
struct number
{
    int sign = 1;
    int exp_sign = 1;
    range<ForwardIterator> integral_part;
    range<ForwardIterator> fract_part;
    range<ForwardIterator> exp_part;
};

////////////////////////////////////////////////////////////////////////////////
// compare_and_assign
////////////////////////////////////////////////////////////////////////////////
/**
 * Helper function assigns @a b to @a a if @a a != @a b.
 */
template <typename ForwardIterator>
inline bool compare_and_assign (ForwardIterator & a, ForwardIterator b)
{
    if (a != b) {
        a = b;
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// is_whitespace
////////////////////////////////////////////////////////////////////////////////
/**
 * @return @c true if character is one of symbols:
 *      - space (%x20),
 *      - horizontal tab (%x09),
 *      - line feed or new line (%x0A),
 *      - carriage return (%x0D),
 *
 *      otherwise @c false.
 */
template <typename CharT>
inline bool is_whitespace (CharT ch)
{
    return (ch == CharT('\x20')
            || ch == CharT('\x09')
            || ch == CharT('\x0A')
            || ch == CharT('\x0D'));
}

////////////////////////////////////////////////////////////////////////////////
// is_digit
////////////////////////////////////////////////////////////////////////////////
/**
 * @return @c true if character is a decimal digit (0..9), otherwise @c false.
 */
template <typename CharT>
inline bool is_digit (CharT ch)
{
    // return (ch >= CharT('0') && ch <= CharT('9'));
    return (ch == CharT('0')
            || ch == CharT('1')
            || ch == CharT('2')
            || ch == CharT('3')
            || ch == CharT('4')
            || ch == CharT('5')
            || ch == CharT('6')
            || ch == CharT('7')
            || ch == CharT('8')
            || ch == CharT('9'));
}

////////////////////////////////////////////////////////////////////////////////
// is_hexdigit
////////////////////////////////////////////////////////////////////////////////
/**
 * @return @c true if character is a hexadecimal digit (0..9A..Fa..f).
 */
template <typename CharT>
inline bool is_hexdigit (CharT ch)
{
//     return is_digit(ch)
//             || (ch >= CharT('A') && ch <= CharT('F'))
//             || (ch >= CharT('a') && ch <= CharT('f'));
    return (is_digit(ch)
            || ch == CharT('a')
            || ch == CharT('b')
            || ch == CharT('c')
            || ch == CharT('d')
            || ch == CharT('e')
            || ch == CharT('f')
            || ch == CharT('A')
            || ch == CharT('B')
            || ch == CharT('C')
            || ch == CharT('D')
            || ch == CharT('E')
            || ch == CharT('F'));
}

////////////////////////////////////////////////////////////////////////////////
// advance_whitespaces
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by sequence of whitespace characters.
 *
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 */
template <typename ForwardIterator>
bool advance_whitespaces (ForwardIterator & pos, ForwardIterator last)
{
    ForwardIterator p = pos;

    while (p != last && is_whitespace(*p))
        ++p;

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_number
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by number.
 *
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 *
 * @note Grammar
 *
 * number = [ minus ] int [ frac ] [ exp ]
 * decimal-point = %x2E       ; .
 * digit1-9 = %x31-39         ; 1-9
 * e = %x65 / %x45            ; e E
 * exp = e [ minus / plus ] 1*DIGIT
 * frac = decimal-point 1*DIGIT
 * int = zero / ( digit1-9 *DIGIT )
 * minus = %x2D               ; -
 * plus = %x2B                ; +
 * zero = %x30                ; 0
 *
 */
template <typename ForwardIterator>
bool advance_number (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy = strict_policy()
        , number<ForwardIterator> * num = nullptr)
{
    ForwardIterator p = pos;
    int sign = 1;
    int exp_sign = 1;

    if (p != last) {
        if (*p == '-') {
            sign = -1;
            ++p;
        } else if (*p == '+') {
            if (parse_policy.test(allow_positive_signed_number)) {
                sign = 1;
                ++p;
            } else {
                return false;
            }
        }
    }

    ForwardIterator last_pos = p;

    ////////////////////////////////////////////////////////////////////////////
    // Integral part
    //
    // Mandatory
    // int = zero / ( digit1-9 *DIGIT )
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == '0') {
            ++p;
        } else {
            while (p != last && is_digit(*p)) {
                ++p;
            }
        }
    }

    // No digit found
    if (p == last_pos)
        return false;

    if (num) {
        num->integral_part.first = last_pos;
        num->integral_part.second = p;
        num->sign = sign;
    }

    last_pos = p;

    ////////////////////////////////////////////////////////////////////////////
    // Fractional part
    //
    // Optional
    // frac = decimal-point 1*DIGIT
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == '.') {
            ++p;

            if (p == last)
                return false;

            if (!is_digit(*p))
                return false;

            ++p;

            while (p != last && is_digit(*p)) {
                ++p;
            }
        }
    }

    if (num) {
        num->fract_part.first = last_pos;
        num->fract_part.second = p;
    }

    last_pos = p;

    ////////////////////////////////////////////////////////////////////////////
    // Exponentional part
    //
    // Optional
    // exp = e [ minus / plus ] 1*DIGIT
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == 'e' || *p == 'E') {
            ++p;

            if (p != last) {
                if (*p == '-') {
                    exp_sign = -1;
                    ++p;
                } else if (*p == '+') {
                    exp_sign = 1;
                    ++p;
                }
            }

            if (p == last)
                return false;

            if (!is_digit(*p))
                return false;

            ++p;

            while (p != last && is_digit(*p)) {
                ++p;
            }
        }
    }

    if (num) {
        num->exp_part.first = last_pos;
        num->exp_part.second = p;
        num->exp_sign = exp_sign;
    }

    return compare_and_assign(pos, p);
}



// static constexpr char begin_array_char     = '\x5B'; // '[' left square bracket
// static constexpr char begin_object_char    = '\x7B'; // '{' left curly bracket
// static constexpr char end_array_char       = '\x5D'; // ']' right square bracket
// static constexpr char end_object_char      = '\x7D'; // '}' right curly bracket
// static constexpr char name_separator_char  = '\x3A'; // ':' colon
// static constexpr char value_separator_char = '\x2C'; // ',' comma

template <typename ForwardIterator>
bool advance_json (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy = strict_policy()
        , value * val = nullptr)
{
    ForwardIterator p = pos;

    do {
//         if (parse_policy.test(allow_object_root_element)
//                 && advance_object(p, last, val))
//             break;
//
//         if (parse_policy.test(allow_array_root_element)
//                 && advance_array(p, last, val))
//             break;
//
//         if (parse_policy.test(allow_null_root_element)
//                 && advance_boolean(p, last, val))
//             break;

        number<ForwardIterator> num;

        if (parse_policy.test(allow_number_root_element)
                && advance_number(p, last, parse_policy, & num)) {

            if (val) {
                *val = to_number(num);
            }

            break;
        }

//         if (parse_policy.test(allow_string_root_element)
//                 && advance_string(p, last, val))
//             break;
//
//         if (parse_policy.test(allow_boolean_root_element)
//                 && advance_boolean(p, last, val))
//             break;
    } while (false);

    return compare_and_assign(pos, p);
}

template <typename ForwardIterator>
ForwardIterator parse (ForwardIterator first
        , ForwardIterator last
        , parse_policy_set const & parse_policy = strict_policy()
        , value * val = nullptr)
{
    ForwardIterator pos = first;

    if (advance_json(pos, last, val, parse_policy)) {
        return pos;
    }

    return first;
}


}} // // namespace pfs::json
