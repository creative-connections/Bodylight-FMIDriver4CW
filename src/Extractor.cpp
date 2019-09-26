#include "stdafx.h" 
#include "Extractor.hpp"

#include <iostream>
#include "libs/miniz/zip_file.hpp"

void Extractor::extractAll(std::wstring dst, std::wstring src)
{
  miniz_cpp::zip_file source(src);
  source.extractall(dst);
}

Extractor::Extractor()
{
}

Extractor::~Extractor()
{
}



