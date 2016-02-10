/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Database functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef DatabaseH
#define DatabaseH
//---------------------------------------------------------------------------

#include <map>
#include <vector>
#include "MediaConchLib.h"
//---------------------------------------------------------------------------

namespace MediaConch {

//***************************************************************************
// Class Database
//***************************************************************************

class Database
{
public:
    //Constructor/Destructor
    Database();
    virtual ~Database();

    virtual int create_report_table() = 0;
    virtual int update_report_table() = 0;
    virtual int save_report(MediaConchLib::report reportKind, MediaConchLib::format format,
                            const std::string& filename, const std::string& file_last_modification,
                            const std::string& report, MediaConchLib::compression) = 0;
    virtual int remove_report(const std::string& filename) = 0;
    virtual void get_report(MediaConchLib::report reportKind, MediaConchLib::format format,
                            const std::string& filename, const std::string& file_last_modification,
                            std::string& report, MediaConchLib::compression&) = 0;
    virtual bool file_is_registered(MediaConchLib::report reportKind, MediaConchLib::format format,
                                    const std::string& file, const std::string& file_last_modification) = 0;
    virtual void get_elements(std::vector<std::string>& vec) = 0;

    void        set_database_directory(const std::string& dirname);
    void        set_database_filename(const std::string& name);

    // UI
    virtual int ui_add_file(const std::string& filename, const std::string& filepath,
                            int policy, int display, bool analyzed,
                            bool implementation_valid, bool policy_valid) = 0;
    virtual int ui_update_file(const std::string& filename, const std::string& filepath,
                               int policy, int display, bool analyzed,
                               bool implementation_valid, bool policy_valid) = 0;
    virtual int ui_get_file(const std::string& filename, const std::string& filepath,
                            int& policy, int& display, bool& analyzed,
                            bool& implementation_valid, bool& policy_valid) = 0;
    virtual int ui_remove_file(const std::string& filename, const std::string& filepath) = 0;
    virtual void ui_get_elements(std::vector<std::pair<std::string, std::string> >& vec) = 0;

    virtual int init() = 0;
    virtual int init_report() = 0;
    virtual int init_ui() = 0;

    const std::vector<std::string>& get_errors() const { return errors; }

protected:
    std::string                        query;
    std::vector<std::string>           errors;
    std::vector<std::map<std::string, std::string> > reports;
    std::string                        db_filename;
    std::string                        db_dirname;

    //Database dependant
    virtual int execute() = 0;
    void        get_sql_query_for_create_report_table(std::string& q);
    void        get_sql_query_for_update_report_table(std::string& q);
    void        get_sql_query_for_create_ui_table(std::string& q);
    void        get_sql_query_for_update_ui_table(std::string& q);

private:
    Database (const Database&);
    Database& operator=(const Database&);
};

}

#endif
