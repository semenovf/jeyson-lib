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
#include <algorithm>
#include <bitset>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <cassert>
#include <clocale>
#include <cstdlib>

namespace pfs {
namespace json {

/**
 * [RFC 8259](https://tools.ietf.org/html/rfc8259)
 * [RFC 7159](https://tools.ietf.org/html/rfc7159)
 * [RFC 7158](https://tools.ietf.org/html/rfc7158)
 * [RFC 4627](https://tools.ietf.org/html/rfc4627)
 * [JSON5](https://spec.json5.org)
 *
 * History:
 *
 * RFC 8259 obsoletes RFC 7159
 * RFC 7159 obsoletes RFC 7158 and RFC 4627
 * RFC 7158 obsoletes RFC 4627
 */

/* In short the main difference between RFC 8259(7159) and RFC 4627 in one rule:
 *
 * RFC 4627: JSON-text = object / array
 * RFC 8259: JSON-text = ws value ws
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
 *
 *  RFC 7159:
 *  RFC 8259: JSON-text = ws value ws
 *
 *  RFC 4627:
 *  RFC 7159:
 *  RFC 8259:
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
 *  RFC 8259:
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
 *  RFC 8259:
 *  ----------------------------------------------------------------------------
 *  false null true
 *  ----------------------------------------------------------------------------
 *
 *  The literal names MUST be lowercase.  No other literal names are
 *  allowed.
 *
 *  RFC 4627:
 *  RFC 7159:
 *  RFC 8259:
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
 *  RFC 8259:
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
 *  RFC 8259:
 *  ----------------------------------------------------------------------------
 *  array = begin-array [ value *( value-separator value ) ] end-array
 *  ----------------------------------------------------------------------------
 *
 *  Numbers.
 *  ============================================================================
 *  RFC 4627:
 *  RFC 7159:
 *  RFC 8259:
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
 *  RFC 8259:
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
 *
 *  RFC 8259:
 *  ----------------------------------------------------------------------------
 *  A JSON parser transforms a JSON text into another representation.  A
 *  JSON parser MUST accept all texts that conform to the JSON grammar.
 *  A JSON parser MAY accept non-JSON forms or extensions.
 *  An implementation may set limits on the size of texts that it
 *  accepts.  An implementation may set limits on the maximum depth of
 *  nesting.  An implementation may set limits on the range and precision
 *  of numbers.  An implementation may set limits on the length and
 *  character contents of strings.
 *
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

    // Allow apostrophe as quotation mark besides double quote
    , allow_single_quote_mark

    // Allow any escaped character in string not only permitted by grammar
    , allow_any_char_escaped

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

template <typename ForwardIterator>
struct line_counter_iterator
{
    size_t _lineno = 0;
    ForwardIterator _it;
    bool _is_CR = false;

public:
    line_counter_iterator (ForwardIterator initial)
        : _it(initial)
    {}

    line_counter_iterator (line_counter_iterator const & rhs)
        : _lineno(rhs._lineno)
        , _it(rhs._it)
    {}

    line_counter_iterator & operator ++ ()
    {
        if (*_it == '\x0D')
            _is_CR == true;
        else
            _is_CR = false;

        ++_it;

        if (*_it == '\x0D' || (*_it == '\x0A' && !_is_CR))
            ++_lineno;

        return *this;
    }

    line_counter_iterator operator ++ (int)
    {
        line_counter_iterator it(*this);
        ++(*this);
        return it;
    }

    size_t lineno () const
    {
        return _lineno;
    }
};

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
    result.set(allow_single_quote_mark, true);
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
    parse_policy_set result = json5_policy();
    result.set(allow_positive_signed_number, true);
    result.set(allow_any_char_escaped, true);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// locale_decimal_point
////////////////////////////////////////////////////////////////////////////////
/**
 * Get locale-specific decimal point character.
 */
inline char locale_decimal_point ()
{
    std::lconv const * loc = std::localeconv();
    assert(loc);
    return (loc->decimal_point && *loc->decimal_point != '\x0')
            ? *loc->decimal_point : '.';
}

////////////////////////////////////////////////////////////////////////////////
// Convert string to number functions
////////////////////////////////////////////////////////////////////////////////
inline bool strtointeger (long int & n, std::string const & numstr)
{
    char * endptr = nullptr;
    n = std::strtol(numstr.c_str(), & endptr, 10);
    return (errno != ERANGE && endptr == & *numstr.end());
}

inline bool strtointeger (long long int & n, std::string const & numstr)
{
    char * endptr = nullptr;
    n = std::strtoll(numstr.c_str(), & endptr, 10);
    return (errno != ERANGE && endptr == & *numstr.end());
}

inline bool strtouinteger (unsigned long int & n, std::string const & numstr)
{
    char * endptr = nullptr;
    n = std::strtoul(numstr.c_str(), & endptr, 10);
    return (errno != ERANGE && endptr == & *numstr.end());
}

inline bool strtouinteger (unsigned long long int & n, std::string const & numstr)
{
    char * endptr = nullptr;
    n = std::strtoull(numstr.c_str(), & endptr, 10);
    return (errno != ERANGE && endptr == & *numstr.end());
}

inline bool strtoreal (float & n, std::string const & numstr)
{
    char * endptr = nullptr;
    n = std::strtof(numstr.c_str(), & endptr);
    return (errno != ERANGE && endptr == & *numstr.end());
}

inline bool strtoreal (double & n, std::string const & numstr)
{
    char * endptr = nullptr;
    n = std::strtod(numstr.c_str(), & endptr);
    return (errno != ERANGE && endptr == & *numstr.end());
}

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
//
////////////////////////////////////////////////////////////////////////////////
/**
 * @return Base-@a radix digit converted from character @a ch,
 *      or -1 if conversion is impossible. @a radix must be between 2 and 36
 *      inclusive.
 */
template <typename CharT>
int to_digit (CharT ch, int radix = 10)
{
    int digit = 0;

    // Bad radix
    if (radix < 2 || radix > 36)
        return -1;

    if (int(ch) >= int('0') && int(ch) <= int('9'))
        digit = int(ch) - int('0');
    else if (int(ch) >= int('a') && int(ch) <= int('z'))
        digit = int(ch) - int('a') + 10;
    else if (int(ch) >= int('A') && int(ch) <= int('Z'))
        digit = int(ch) - int('A') + 10;
    else
        return -1;

    if (digit >= radix)
        return -1;

    return digit;
}

////////////////////////////////////////////////////////////////////////////////
// is_quotation_mark
////////////////////////////////////////////////////////////////////////////////
template <typename CharT>
inline bool is_quotation_mark (CharT ch, parse_policy_set const & parse_policy)
{
    return (ch == '"'
            || (parse_policy.test(allow_single_quote_mark) && ch == '\''));
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
bool advance_whitespaces (ForwardIterator & pos
        , ForwardIterator last)
{
    ForwardIterator p = pos;

    while (p != last && is_whitespace(*p))
        ++p;

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_sequence
// Based on pfs/algo/advance.hpp:advance_sequence
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by sequence of charcters.
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by all character sequence [first2, last2),
 *      otherwise returns @c false.
 */
template <typename ForwardIterator1, typename ForwardIterator2>
inline bool advance_sequence (ForwardIterator1 & pos, ForwardIterator1 last
        , ForwardIterator2 first2, ForwardIterator2 last2)
{
    ForwardIterator1 p = pos;

    while (p != last && first2 != last2 && *p++ == *first2++)
        ;

    if (first2 == last2) {
        pos = p;
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// advance_null
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by 'null' string.
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 *
 * @note Grammar
 * null  = %x6e.75.6c.6c      ; null
 */
template <typename ForwardIterator>
inline bool advance_null (ForwardIterator & pos, ForwardIterator last)
{
    std::string s{"null"};
    return advance_sequence(pos, last, s.begin(), s.end());
}

////////////////////////////////////////////////////////////////////////////////
// advance_true
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by 'true' string.
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 *
 * @note Grammar
 * true  = %x74.72.75.65      ; true
 */
template <typename ForwardIterator>
inline bool advance_true (ForwardIterator & pos, ForwardIterator last)
{
    std::string s{"true"};
    return advance_sequence(pos, last, s.begin(), s.end());
}

////////////////////////////////////////////////////////////////////////////////
// advance_false
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by 'false' string.
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 *
 * @note Grammar
 * true  = %x74.72.75.65      ; true
 */
template <typename ForwardIterator>
inline bool advance_false (ForwardIterator & pos, ForwardIterator last)
{
    std::string s{"false"};
    return advance_sequence(pos, last, s.begin(), s.end());
}

////////////////////////////////////////////////////////////////////////////////
// advance_encoded_char
////////////////////////////////////////////////////////////////////////////////
/**
 * @note Grammar:
 * encoded_char = 4HEXDIG
 */
template <typename ForwardIterator>
bool advance_encoded_char (ForwardIterator & pos, ForwardIterator last
        , int32_t & result)
{
    static int32_t multipliers[] = { 16 * 16 * 16, 16 * 16, 16, 1 };
    static int count = sizeof(multipliers) / sizeof(multipliers[0]);
    ForwardIterator p = pos;
    int index = 0;

    result = 0;

    for (p = pos; p != last && is_hexdigit(*p) && index < count; ++p, ++index) {
        int32_t n = to_digit(*p, 16);
        result += n * multipliers[index];
    }

    if (index != count)
        return false;

    return compare_and_assign(pos, p);
}


////////////////////////////////////////////////////////////////////////////////
// advance_string
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by string.
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 *
 * @note Grammar
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
 *      / %x27                 ; '  JSON5 specific
 *  unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
 */
template <typename ForwardIterator, typename StringType>
inline bool advance_string (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , StringType & result
        , error_code & ec)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;

    ForwardIterator p = pos;

    if (p == last)
        return false;

    if (!is_quotation_mark(*p, parse_policy))
        return false;

    auto quotation_mark = *p;

    ++p;

    if (p == last) {
        ec = make_error_code(errc::unbalanced_quote);
        return false;
    }

    // Check empty string
    if (*p == quotation_mark) {
        ++p;
        return compare_and_assign(pos, p);
    }

    bool escaped = false;
    bool encoded = false;

    auto output = std::back_inserter(result);

    while (p != last && *p != quotation_mark) {
        if (encoded) {
            int32_t encoded_char = 0;

            if (!advance_encoded_char(p, last, encoded_char)) {
                ec = make_error_code(errc::bad_encoded_char);
                return false;
            }

            *output++ = char_type(encoded_char);

            encoded = false;
            continue;
        }

        if (!escaped) {
            if (*p == '\\') { // escape character
                escaped = true;
            } else {
                *output++ = *p;
            }
        } else {
            auto escaped_char = *p;

            switch (escaped_char) {
                case '"':
                case '\\':
                case '/': break;

                case '\'':
                    if (quotation_mark != '\'') {
                        ec = make_error_code(errc::bad_escaped_char);
                        return false;
                    }
                    break;

                case 'b': escaped_char = '\b'; break;
                case 'f': escaped_char = '\f'; break;
                case 'n': escaped_char = '\n'; break;
                case 'r': escaped_char = '\r'; break;
                case 't': escaped_char = '\t'; break;

                case 'u': encoded = true; break;

                default:
                    if (!parse_policy.test(allow_any_char_escaped)) {
                        ec = make_error_code(errc::bad_escaped_char);
                        return false;
                    }
            }

            if (!encoded) {
                *output++ = escaped_char;
            }

            // Finished process escaped sequence
            escaped = false;
        }

        ++p;
    }

    // ERROR: unquoted string
    if (p == last || *p != quotation_mark) {
        ec = make_error_code(errc::unbalanced_quote);
        return false;
    }

    // Skip quotation mark
    ++p;

    return compare_and_assign(pos, p);
}


////////////////////////////////////////////////////////////////////////////////
// advance_number
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Advance by number.
 * @param pos On input - first position, on output - last good position.
 * @param last End of sequence position.
 * @return @c true if advanced by at least one position, otherwise @c false.
 *
 * @note Grammar
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
 * @a NumberRep traits
 *
 * NumberRep::operator = (double);
 * NumberRep::operator = (intmax_t);
 * NumberRep::operator = (uintmax_t);
 */
template <typename ForwardIterator, typename NumberType>
bool advance_number (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , NumberType & num
        , error_code & /*ec*/)
{
    ForwardIterator p = pos;
    std::string numstr;
    int sign = 1;
    bool is_integer = true;

    ////////////////////////////////////////////////////////////////////////////
    // Advance sign
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == '-') {
            sign = -1;
            numstr.push_back('-');
            ++p;
        } else if (*p == '+') {
            if (parse_policy.test(allow_positive_signed_number)) {
                ++p;
            } else {
                return false;
            }
        }
    }

    ForwardIterator last_pos = p;

    ////////////////////////////////////////////////////////////////////////////
    // Advance integral part
    //
    // Mandatory
    // int = zero / ( digit1-9 *DIGIT )
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == '0') {
            numstr.push_back('0');
            ++p;
        } else {
            while (p != last && is_digit(*p)) {
                numstr.push_back(*p);
                ++p;
            }
        }
    }

    // No digit found
    if (p == last_pos)
        return false;

    last_pos = p;

    ////////////////////////////////////////////////////////////////////////////
    // Fractional part
    //
    // Optional
    // frac = decimal-point 1*DIGIT
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == '.') {
            is_integer = false;
            numstr.push_back(locale_decimal_point());

            ++p;

            if (p == last)
                return false;

            if (!is_digit(*p))
                return false;

            numstr.push_back(*p);
            ++p;

            while (p != last && is_digit(*p)) {
                numstr.push_back(*p);
                ++p;
            }
        }
    }

    last_pos = p;

    ////////////////////////////////////////////////////////////////////////////
    // Exponential part
    //
    // Optional
    // exp = e [ minus / plus ] 1*DIGIT
    ////////////////////////////////////////////////////////////////////////////
    if (p != last) {
        if (*p == 'e' || *p == 'E') {
            is_integer = false;
            numstr.push_back('e');
            ++p;

            if (p != last) {
                if (*p == '-') {
                    numstr.push_back('-');
                    ++p;
                } else if (*p == '+') {
                    ++p;
                }
            }

            if (p == last)
                return false;

            if (!is_digit(*p))
                return false;

            numstr.push_back(*p);
            ++p;

            while (p != last && is_digit(*p)) {
                numstr.push_back(*p);
                ++p;
            }
        }
    }

    bool integer_accepted = false;

    if (is_integer) {
        if (sign > 0) {
            uintmax_t n = 0;

            if (strtouinteger(n, numstr)) {
                num = n;
                integer_accepted = true;
            }
        } else {
            intmax_t n = 0;

            if (strtointeger(n, numstr)) {
                num = n;
                integer_accepted = true;
            }
        }
    }

    if (!integer_accepted) {
        double n = 0;
        if (!strtoreal(n, numstr)) {
            return false;
        }

        num = n;
    }

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// Forward declaration for advance_value
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator, typename ValueType>
typename std::enable_if<std::is_arithmetic<ValueType>::value, bool>::type
advance_value (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueType & value
        , error_code & ec);

// std::string
template <typename ForwardIterator, typename ValueType>
typename std::enable_if<std::is_same<ValueType, std::string>::value, bool>::type
advance_value (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueType & value
        , error_code & ec);

template <typename ForwardIterator, typename ValueType>
typename std::enable_if<!std::is_same<ValueType, std::string>::value && !std::is_arithmetic<ValueType>::value, bool>::type
advance_value (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueType & value
        , error_code & ec);

////////////////////////////////////////////////////////////////////////////////
// advance_value_separator
////////////////////////////////////////////////////////////////////////////////
/**
 *  begin-array     = ws %x5B ws  ; [ left square bracket
 *  begin-object    = ws %x7B ws  ; { left curly bracket
 *  end-array       = ws %x5D ws  ; ] right square bracket
 *  end-object      = ws %x7D ws  ; } right curly bracket
 *  name-separator  = ws %x3A ws  ; : colon
 *  value-separator = ws %x2C ws  ; , comma
 */
template <typename ForwardIterator>
bool advance_delimiter_char (ForwardIterator & pos, ForwardIterator last
        , typename std::remove_reference<decltype(*pos)>::type delim)
{
    ForwardIterator p = pos;

    advance_whitespaces(p, last);

    if (p == last)
        return false;

    if (*p != delim)
        return false;

    ++p;
    advance_whitespaces(p, last);

    return compare_and_assign(pos, p);
}

template <typename ForwardIterator>
inline bool advance_begin_array (ForwardIterator & pos, ForwardIterator last)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;
    return advance_delimiter_char(pos, last, char_type{'['});
}

template <typename ForwardIterator>
inline bool advance_begin_object (ForwardIterator & pos, ForwardIterator last)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;
    return advance_delimiter_char(pos, last, char_type{'{'});
}

template <typename ForwardIterator>
inline bool advance_end_array (ForwardIterator & pos, ForwardIterator last)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;
    return advance_delimiter_char(pos, last, char_type{']'});
}

template <typename ForwardIterator>
inline bool advance_end_object (ForwardIterator & pos, ForwardIterator last)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;
    return advance_delimiter_char(pos, last, char_type{'}'});
}

template <typename ForwardIterator>
inline bool advance_name_separator (ForwardIterator & pos, ForwardIterator last)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;
    return advance_delimiter_char(pos, last, char_type{':'});
}

template <typename ForwardIterator>
inline bool advance_value_separator (ForwardIterator & pos, ForwardIterator last)
{
    using char_type = typename std::remove_reference<decltype(*pos)>::type;
    return advance_delimiter_char(pos, last, char_type{','});
}

////////////////////////////////////////////////////////////////////////////////
// advance_array
////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @note Grammar:
 * array = begin-array [ value *( value-separator value ) ] end-array
 * begin-array     = ws %x5B ws  ; [ left square bracket
 * end-array       = ws %x5D ws  ; ] right square bracket
 * value-separator = ws %x2C ws  ; , comma
 * value = false / null / true / object / array / number / string
 */
template <typename ForwardIterator, typename ArrayType>
bool advance_array (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ArrayType & array
        , error_code & ec)
{
    ForwardIterator p = pos;

    if (!advance_begin_array(p, last))
        return false;

    do {
        typename ArrayType::value_type item;

        if (!advance_value(p, last, parse_policy, item, ec)) {
            // Error while parsing value
            if (ec)
                return false;

            break;
        }

        array.emplace_back(std::move(item));
    } while(advance_value_separator(p, last));

    if (!advance_end_array(p, last)) {
        ec = make_error_code(errc::unbalanced_array_bracket);
        return false;
    }

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_member
////////////////////////////////////////////////////////////////////////////////
/**
 * @note Grammar:
 * member = string name-separator value
 * name-separator  = ws %x3A ws  ; : colon
 * value = false / null / true / object / array / number / string
 */
template <typename ForwardIterator, typename ValueType>
bool advance_member (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , error_code & ec)
{
    ForwardIterator p = pos;

    typename ValueType::string_type name;

    if (!advance_string(p, last, parse_policy, name, ec))
        return false;

    if (!advance_name_separator(p, last))
        return false;

    ValueType value;

    if (!advance_value(p, last, parse_policy, value, ec))
        return false;

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_object
////////////////////////////////////////////////////////////////////////////////
/**
 * @details ObjectType traits:
 *  - provides @code key_type @endcode typename
 *  - provides @code mapped_type @endcode typename
 *  - provides @code emplace(std::pair<key_type, mapped_type> &&) @endcode method
 *
 * @note Grammar:
 * object = begin-object [ member *( value-separator member ) ] end-object
 * member = string name-separator value
 * begin-object    = ws %x7B ws  ; { left curly bracket
 * end-object      = ws %x7D ws  ; } right curly bracket
 * name-separator  = ws %x3A ws  ; : colon
 * value-separator = ws %x2C ws  ; , comma
 * value = false / null / true / object / array / number / string
 */
template <typename ForwardIterator, typename ObjectType>
bool advance_object (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ObjectType & object
        , error_code & ec)
{
    ForwardIterator p = pos;

    if (!advance_begin_object(p, last))
        return false;

    do {
        typename ObjectType::key_type name;

        if (!advance_string(p, last, parse_policy, name, ec)) {
            // Error while parsing value
            if (ec)
                return false;

            break;
        }

        if (!advance_name_separator(p, last))
            return false;

        typename ObjectType::mapped_type value;

        if (!advance_value(p, last, parse_policy, value, ec))
            return false;

        object.emplace(std::make_pair(std::move(name), std::move(value)));
    } while(advance_value_separator(p, last));

    if (!advance_end_object(p, last)) {
        ec = make_error_code(errc::unbalanced_array_bracket);
        return false;
    }

    return compare_and_assign(pos, p);
}

// static constexpr char begin_array_char     = '\x5B'; // '[' left square bracket
// static constexpr char begin_object_char    = '\x7B'; // '{' left curly bracket
// static constexpr char end_array_char       = '\x5D'; // ']' right square bracket
// static constexpr char end_object_char      = '\x7D'; // '}' right curly bracket
// static constexpr char name_separator_char  = '\x3A'; // ':' colon
// static constexpr char value_separator_char = '\x2C'; // ',' comma

////////////////////////////////////////////////////////////////////////////////
// advance_value
//
// Specialization: yes, for values of arithmetic type
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator, typename ValueType>
typename std::enable_if<std::is_arithmetic<ValueType>::value, bool>::type
advance_value (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueType & value
        , error_code & ec)
{
    ForwardIterator p = pos;

    // Skip head witespaces
    advance_whitespaces(p, last);

    do {
        ValueType num;

        if (advance_number(p, last, parse_policy, num, ec)) {
            value = num;
            break;
        }

        // Not an array of values of arithmetic type
        return false;
    } while (false);

    // Skip tail witespaces
    advance_whitespaces(p, last);

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_value
//
// Specialization: yes, for values of std::string
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator, typename ValueType>
typename std::enable_if<std::is_same<ValueType, std::string>::value, bool>::type
advance_value (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueType & value
        , error_code & ec)
{
    ForwardIterator p = pos;

    // Skip head witespaces
    advance_whitespaces(p, last);

    do {
        ValueType str;

        if (advance_string(p, last, parse_policy, str, ec)) {
            value = std::move(str);
            break;
        }

        // Not an array of std::string values
        return false;
    } while (false);

    // Skip tail witespaces
    advance_whitespaces(p, last);

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_value
//
// Specialization: no, for values of any type
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator, typename ValueType>
typename std::enable_if<!std::is_same<ValueType, std::string>::value && !std::is_arithmetic<ValueType>::value, bool>::type
advance_value (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueType & value
        , error_code & ec)
{
    ForwardIterator p = pos;

    // Skip head witespaces
    advance_whitespaces(p, last);

    do {
        // Array marker
        if (*p == '[') {
            typename ValueType::array_type array;

            if (advance_array(p, last, parse_policy, array, ec)) {
                value = std::move(array);
                break;
            }
        }

        // Object marker
        if (*p == '{') {
            typename ValueType::object_type object;

            if (advance_object(p, last, parse_policy, object, ec)) {
                value = std::move(object);
                break;
            }
        }

        if (advance_null(p, last)) {
            value = nullptr;
            break;
        }

        if (advance_true(p, last)) {
            value = true;
            break;
        }

        if (advance_false(p, last)) {
            value = false;
            break;
        }

        typename ValueType::number_type num;

        if (advance_number(p, last, parse_policy, num, ec)) {
            value = num;
            break;
        }

        typename ValueType::string_type str;

        if (advance_string(p, last, parse_policy, str, ec)) {
            value = std::move(str);
            break;
        }

        // Not JSON sequence
        return false;
    } while (false);

    // Skip tail witespaces
    advance_whitespaces(p, last);

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
// advance_json
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator, typename JsonType>
bool advance_json (ForwardIterator & pos, ForwardIterator last
        , parse_policy_set const & parse_policy
        , JsonType & value
        , error_code & ec)
{
    ForwardIterator p = pos;

    // Skip head witespaces
    advance_whitespaces(p, last);

    if ((*p == '[' && !parse_policy.test(allow_array_root_element))
            || (*p == '{' && !parse_policy.test(allow_object_root_element))) {
        ec = make_error_code(errc::forbidden_root_element);
        return false;
    }

    if (!advance_value(p, last, parse_policy, value, ec))
        return false;

    if ((value.is_null() && !parse_policy.test(allow_array_root_element))
            || (value.is_boolean() && !parse_policy.test(allow_boolean_root_element))
            || (value.is_number() && !parse_policy.test(allow_number_root_element))
            || (value.is_string() && !parse_policy.test(allow_string_root_element))
            || (value.is_array() && !parse_policy.test(allow_array_root_element))
            || (value.is_object() && !parse_policy.test(allow_object_root_element))) {
        ec = make_error_code(errc::forbidden_root_element);
        return false;
    }

    // Skip tail witespaces
    advance_whitespaces(p, last);

    return compare_and_assign(pos, p);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
template <typename ForwardIterator, typename ValueRep>
inline ForwardIterator parse (ForwardIterator first
        , ForwardIterator last
        , parse_policy_set const & parse_policy
        , ValueRep & value)
{
    ForwardIterator pos = first;

    if (advance_json(pos, last, parse_policy, value)) {
        return pos;
    }

    return first;
}

/**
 *
 */
template <typename ForwardIterator, typename ValueRep>
inline ForwardIterator parse (ForwardIterator first
        , ForwardIterator last
        , ValueRep & value)
{
    return parse(first, last, strict_policy(), value);
}

}} // // namespace pfs::json
