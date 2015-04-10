/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       util.cc
 * OVERVIEW:   This file contains miscellaneous functions that don't belong to
 *             any particular subsystem of UQBT.
 *============================================================================*/

#include "util.h"

#include <unistd.h>
#include <fcntl.h>

#include <iomanip>          // For setw
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <cstdio>
#include <cstring>

/*==============================================================================
 * FUNCTION:      string::operator+(string, int)
 * OVERVIEW:      Append an int to a string
 * PARAMETERS:    s: the string to append to
 *                i: the integer whose ascii representation is to be appended
 * RETURNS:       A copy of the modified string
 *============================================================================*/
std::string operator+(const std::string &s, int i)
{
	static char buf[50];
	std::string ret(s);

	sprintf(buf, "%d", i);
	return ret.append(buf);
}

/*==============================================================================
 * FUNCTION:      initCapital
 * OVERVIEW:      Return a string the same as the input string, but with the
 *                  first character capitalised
 * PARAMETERS:    s: the string to capitalise
 * RETURNS:       A copy of the modified string
 *============================================================================*/
std::string initCapital(const std::string &s)
{
	std::string res(s);
	res[0] = toupper(res[0]);
	return res;
}

/*==============================================================================
 * FUNCTION:      hasExt
 * OVERVIEW:      Returns true if the given file name has the given extension
 *                and false otherwise.
 * PARAMETERS:    s: string representing a file name (e.g. string("foo.c"))
 *                e: the extension (e.g. ".o")
 * RETURNS:       Boolean indicating whether the file name has the extension.
 *============================================================================*/
bool hasExt(const std::string &s, const char *ext)
{
	std::string tailStr = std::string(".") + std::string(ext);
	size_t i = s.rfind(tailStr);
	if (i == std::string::npos) {
		return false;
	} else {
		size_t sLen = s.length();
		size_t tailStrLen = tailStr.length();
		return ((i + tailStrLen) == sLen);
	}
}

/*==============================================================================
 * FUNCTION:      changeExt
 * OVERVIEW:      Change the extension of the given file name
 * PARAMETERS:    s: string representing the file name to be modified
 *                  (e.g. string("foo.c"))
 *                e: the new extension (e.g. ".o")
 * RETURNS:       The converted string (e.g. "foo.o")
 *============================================================================*/
std::string changeExt(const std::string &s, const char *ext)
{
	size_t i = s.rfind(".");
	if (i == std::string::npos) {
		return s + ext;
	} else {
		return s.substr(0, i) + ext;
	}
}

/*==============================================================================
 * FUNCTION:      searchAndReplace
 * OVERVIEW:      returns a copy of a string will all occurances of match
 *                replaced with rep. (simple version of s/match/rep/g)
 * PARAMETERS:    in: the source string
 *                match: the search string
 *                rep: the string to replace match with.
 * RETURNS:       The updated string.
 *============================================================================*/
std::string searchAndReplace(const std::string &in, const std::string &match,
                             const std::string &rep)
{
	std::string result;
	size_t l, n = 0;
	while ((l = in.find(match, n)) != std::string::npos) {
		result.append(in.substr(n, l - n));
		result.append(rep);
		l += match.length();
		n = l;
	}
	result.append(in.substr(n));
	return result;
}

/*==============================================================================
 * FUNCTION:      upperStr
 * OVERVIEW:      Uppercase a C string
 * PARAMETERS:    s: the string (char*) to start with
 *                d: the string (char*) to write to (can be the same string)
 * RETURNS:       Nothing; the string is modified as a side effect
 *============================================================================*/
void upperStr(const char *s, char *d)
{
	size_t len = strlen(s);
	for (size_t i = 0; i < len; ++i)
		d[i] = toupper(s[i]);
	d[len] = '\0';
}

int lockFileRead(const char *fname)
{
	int fd = open("filename", O_RDONLY);  /* get the file descriptor */
	struct flock fl;
	fl.l_type   = F_RDLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
	fl.l_start  = 0;        /* Offset from l_whence         */
	fl.l_len    = 0;        /* length, 0 = to EOF           */
	fl.l_pid    = getpid(); /* our PID                      */
	fcntl(fd, F_SETLKW, &fl);  /* set the lock, waiting if necessary */
	return fd;
}

int lockFileWrite(const char *fname)
{
	int fd = open("filename", O_WRONLY);  /* get the file descriptor */
	struct flock fl;
	fl.l_type   = F_WRLCK;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
	fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
	fl.l_start  = 0;        /* Offset from l_whence         */
	fl.l_len    = 0;        /* length, 0 = to EOF           */
	fl.l_pid    = getpid(); /* our PID                      */
	fcntl(fd, F_SETLKW, &fl);  /* set the lock, waiting if necessary */
	return fd;
}

void unlockFile(int fd)
{
	struct flock fl;
	fl.l_type   = F_UNLCK;  /* tell it to unlock the region */
	fcntl(fd, F_SETLK, &fl); /* set the region to unlocked */
	close(fd);
}

void escapeXMLChars(std::string &s)
{
	std::string bad = "<>&";
	const char *replace[] = { "&lt;", "&gt;", "&amp;" };
	for (size_t i = 0; i < s.size(); ++i) {
		size_t n = bad.find(s[i]);
		if (n != std::string::npos) {
			s.replace(i, 1, replace[n]);
		}
	}
}

// Turn things like newline, return, tab into \n, \r, \t etc
// Note: assumes a C or C++ back end...
char *escapeStr(const char *str)
{
	std::ostringstream out;
	const char unescaped[] = "ntvbrfa\"";
	const char escaped[] = "\n\t\v\b\r\f\a\"";
	bool escapedSucessfully;

	// test each character
	for (; *str; ++str) {
		if (isprint(*str) && *str != '\"') {
			// it's printable, so just print it
			out << *str;
		} else { // in fact, this shouldn't happen, except for "
			// maybe it's a known escape sequence
			escapedSucessfully = false;
			for (size_t i = 0; escaped[i] && !escapedSucessfully; ++i) {
				if (*str == escaped[i]) {
					out << "\\" << unescaped[i];
					escapedSucessfully = true;
				}
			}
			if (!escapedSucessfully) {
				// it isn't so just use the \xhh escape
				out << "\\x" << std::hex << std::setfill('0') << std::setw(2) << (int)*str;
				out << std::setfill(' ');
			}
		}
	}

	char *ret = new char[out.str().size() + 1];
	strcpy(ret, out.str().c_str());
	return ret;
}
