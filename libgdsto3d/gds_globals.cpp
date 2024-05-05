//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2020 Bertrand Pigeard
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
/*
 * File: gds_globals.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This file contains non-class functions.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include "gds_globals.h"
#include "string"

// For the console output
#ifdef WIN32
	#include <filesystem>
    #define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
    #include <experimental/filesystem>
    #include <Windows.h>
	#include <WinBase.h>
	#define OSSEP "\\"
#else
	#define OSSEP "/"
	#include <sys/stat.h>
#endif

int verbose_output=1; // Is this also initialized elsewhere?

void v_printf(const int level, const char *fmt, ...)
{
	if(verbose_output>=level){
		va_list va;
		va_start(va, fmt);
		
#ifndef WIN32
#ifndef __APPLE__
		if(level < 0) // Fatal error to stderr, do only for linux
			vfprintf(stderr, fmt, va);
		else
#endif
#endif
			vprintf(fmt, va);

#ifdef WIN32
		char buf[1024];
		vsprintf(buf, fmt, va);

		size_t len = strlen(buf)+1;
		wchar_t *wText = new wchar_t[len];
		if ( wText == 0 )
			return;
		memset(wText,0,len);
		::MultiByteToWideChar(  CP_ACP, NULL,buf, -1, wText,(int) len );


		OutputDebugString(wText);
		HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
		unsigned long cChars;
		if (std_out == INVALID_HANDLE_VALUE) {
			return ;
		}
		AttachConsole(ATTACH_PARENT_PROCESS);
		if (!WriteConsole(std_out, wText, len, &cChars, NULL)) {
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, wText, len, NULL);
		}

#endif
		va_end(va);
	}
}

char* remove_extension(const char *filename) {
  std::string filenameStr;
  filenameStr = std::string(filename);
  return remove_extension( filenameStr );
}

char* remove_extension(const std::string& filename) {
	std::string str;
	char* ans = new char[1024];
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos) {
		std::copy(filename.begin(), filename.end(), ans);
		ans[filename.size()] = '\0';
	}
	else {
		str = filename.substr(0, lastdot).c_str();
		std::copy(str.begin(), str.end(), ans);
		ans[str.size()] = '\0';
	}
	return ans;
}

char* getfilename(const char *filename) {
  std::string filenameStr;
  filenameStr = std::string(filename);
  return getfilename( filenameStr );
}

char* getfilename(const std::string& fullpathfilename) {
	std::string str;
	char* ans = new char[1024];
	size_t lastslah = fullpathfilename.find_last_of(OSSEP);
	if (lastslah == std::string::npos) {
		lastslah = fullpathfilename.find_last_of("/");
	}
	if (lastslah == std::string::npos) {
		std::copy(fullpathfilename.begin(), fullpathfilename.end(), ans);
		ans[fullpathfilename.size()] = '\0';
	}
	else {
		str = fullpathfilename.substr(lastslah + 1, fullpathfilename.length());
		std::copy(str.begin(), str.end(), ans);
		ans[str.size()] = '\0';
	}
	return ans;
}

typedef struct stat Stat;
#if defined(WIN32) && !defined(__CYGWIN__)
namespace fs = std::experimental::filesystem;
#endif
int do_mkdir(const char *path)
{
	Stat            st;
	int             status = 0;

	if (stat(path, &st) != 0)
	{
		/* Directory does not exist. EEXIST for race condition */
#if defined(WIN32) && !defined(__CYGWIN__)
		if (fs::create_directory(path) != 0 && errno != EEXIST)
			status = -1;
#else
		if (mkdir(path, 0777) != 0)
			status = - 1;
#endif

	}
	return(status);
}

void strCleanUp(char* String) {
	// String CleanUp
	if (String[strlen(String) - 1] == '\n' || String[strlen(String) - 1] == '\r')
		String[strlen(String) - 1] = '\0';
	if (String[strlen(String) - 1] == '\n' || String[strlen(String) - 1] == '\r')
		String[strlen(String) - 1] = '\0';
	while (String[strlen(String) - 1] == ' ')
		String[strlen(String) - 1] = '\0';
}



string StrCalc::toPostfix(char* source)
{
	stack<char> symbol;
	string postfix = "";
	char variables[] = { "0123456789." };
	bool find = false;

	while (*source != '\0')
	{
		switch (*source)
		{
		case '+':
		case '-':
		case '*':
		case '/':
			symbol.push(*source);
			postfix += " ";
			break;
		case ')':
			postfix += symbol.top();
			symbol.pop();
		default:
			find = StrCalc::contain(variables, *source);
			if (find)
			{
				postfix += *source;
				//find = false;
			}

		}
		source++;
	}
	// attach other operator in stack(Pop All)
	while (!symbol.empty())
	{
		postfix += " ";
		postfix += symbol.top();
		symbol.pop();
	}
	return postfix;
	/*char* p = new char[postfix.length()+1];
	const char* o = postfix.c_str();
	for (size_t i = 0; i < postfix.length(); i++)
		p[i] = o[i];
	if (postfix.length() > 0) {
		p[postfix.length()] = *" ";
		p[postfix.length()+1] = '\0';
	}
	return p;
	*/
}

double StrCalc::calculatePostfix(string postfix)
{

	char *source = new char[postfix.length() + 2];
	const char* o = postfix.c_str();
	for (size_t i = 0; i < postfix.length(); i++)
		source[i] = o[i];
	if (postfix.length() > 0) {
		source[postfix.length()] = *" ";
		source[postfix.length()+1] = '\0';
	}
	

	char numbers[] = { "0123456789." };
	stack<double> number;
	int StartIndex = -1;
	char *Number_Str = new char[strlen(source)+1];

	for (size_t i = 0; i < strlen(source); i++)
	{
		char character = source[i];
		if (StrCalc::contain(numbers, character))
		{
			if (StartIndex == -1)
				StartIndex = i;
			//number.push(atof(&character));
		}
		else if (StrCalc::contain((char*)" +-/*", character)) {
			if (StartIndex > -1) {
				
				strncpy(Number_Str, source + StartIndex, i - StartIndex);
				Number_Str[i - StartIndex] = '\0';
				number.push(atof(Number_Str));
				StartIndex = -1;
			}
		}
		if (StrCalc::contain((char*)"+-/*", character))
		{
			double number1, number2;
			switch (character)
			{
			case '+':
				number2 = number.top();
				number.pop();
				if (number.size() == 0) {
					// - alone
					number1 = 0;
				}
				else {
					number1 = number.top();
					number.pop();
				}
				number.push(number1 + number2);
				break;
			case '-':
				number2 = number.top();
				number.pop();
				if (number.size() == 0) {
					// - alone
					number1 = 0;
				} else {
					number1 = number.top();
					number.pop();
				}
				number.push(number1 - number2);
				break;
			case '*':
				number2 = number.top();
				number.pop();
				number1 = number.top();
				number.pop();
				number.push(number1 * number2);
				break;
			case '/':
				number2 = number.top();
				number.pop();
				number1 = number.top();
				number.pop();
				number.push(number1 / number2);
				break;
			}
		}
	}
	delete [] Number_Str;
	delete [] source;
	return number.top();
}

bool StrCalc::contain(char* source, char character)
{
	size_t size = strlen(source);
	for (size_t i = 0; i < size; i++)
	{
		if (source[i] == character)
			return true;
	}
	return false;
}

double GetLineValue(char * line, char *ID) {
	char *Expr_Str;
	double ans;

	Expr_Str = new char[strlen(line) - strlen(ID) + 1];
	strcpy(Expr_Str, line + strlen(ID));
	ans = StrCalc::calculatePostfix(StrCalc::toPostfix(Expr_Str));
	delete[] Expr_Str;
	return ans;
}
