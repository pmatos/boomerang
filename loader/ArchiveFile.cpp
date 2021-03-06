/**
 * \file
 * \brief Contains the implementation of the ArchiveFile class.
 *
 * \authors
 * Copyright (C) 1998, The University of Queensland
 *
 * \copyright
 * See the file "LICENSE.TERMS" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "global.h"

ArchiveFile::ArchiveFile()
{
}

ArchiveFile::~ArchiveFile()
{
}

int ArchiveFile::GetNumMembers() const
{
	return m_FileMap.size();
}

const char *ArchiveFile::GetMemberFileName(int i) const
{
	return m_FileNames[i];
}

BinaryFile *ArchiveFile::GetMemberByProcName(const string &sSym)
{
	// Get the index
	int idx = m_SymMap[sSym];
	// Look it up
	return GetMember(idx);
}

BinaryFile *ArchiveFile::GetMemberByFileName(const string &sFile)
{
	// Get the index
	int idx = m_FileMap[sFile];
	// Look it up
	return GetMember(idx);
}

bool ArchiveFile::PostLoadMember(BinaryFile *pBF, void *handle)
{
	return pBF->PostLoad(handle);
}
