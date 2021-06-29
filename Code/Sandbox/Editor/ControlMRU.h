/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#pragma once
#ifndef CRYINCLUDE_EDITOR_CONTROLMRU_H
#define CRYINCLUDE_EDITOR_CONTROLMRU_H

class CControlMRU
    : public CXTPControlRecentFileList
{
protected:
    virtual void OnCalcDynamicSize(DWORD dwMode);

private:
    DECLARE_XTP_CONTROL(CControlMRU)
    bool DoesFileExist(CString& sFileName);
};
#endif // CRYINCLUDE_EDITOR_CONTROLMRU_H
