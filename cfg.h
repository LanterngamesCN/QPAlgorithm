//
// Created by andy_ro@qq.com
// 			5/26/2019
//
#ifndef CFG_H
#define CFG_H

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <list>
#include <vector>

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>

//按行读取文件
void readFile(char const* filename, std::vector<std::string>& lines, char const* skip);

#endif		 


