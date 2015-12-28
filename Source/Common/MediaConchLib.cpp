/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//---------------------------------------------------------------------------
#include "ZenLib/Ztring.h"
#include "MediaConchLib.h"
#include "Core.h"
#include "DaemonClient.h"
#include "Policy.h"
#include "Http.h"
#include "LibEventHttp.h"

namespace MediaConch {

//***************************************************************************
// Statics
//***************************************************************************

//---------------------------------------------------------------------------
const std::string MediaConchLib::display_xml_name = std::string("XML");
const std::string MediaConchLib::display_maxml_name = std::string("MAXML");
const std::string MediaConchLib::display_text_name = std::string("TEXT");
const std::string MediaConchLib::display_html_name = std::string("HTML");
const std::string MediaConchLib::display_jstree_name = std::string("JSTREE");

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
MediaConchLib::MediaConchLib(bool no_daemon) : use_daemon(false), force_no_daemon(false), daemon_client(NULL)
{
    if (no_daemon)
        force_no_daemon = true;
    core = new Core;
}

MediaConchLib::~MediaConchLib()
{
    delete core;
}

//***************************************************************************
// General
//***************************************************************************

//---------------------------------------------------------------------------
int MediaConchLib::init()
{
    load_configuration();
    if (!force_no_daemon)
    {
        use_daemon = core->is_using_daemon();
        if (use_daemon)
        {
            daemon_client = new DaemonClient(this);
            if (daemon_client->init() < 0)
            {
                // Fallback to no daemon
                use_daemon = false;
            }
        }
    }
    return 0;
}

//---------------------------------------------------------------------------
int MediaConchLib::close()
{
    if (daemon_client)
    {
        daemon_client->close();
        daemon_client = NULL;
    }
    return 0;
}

//***************************************************************************
// Options
//***************************************************************************

//---------------------------------------------------------------------------
int MediaConchLib::add_option(const std::string& option, std::string& report)
{
    size_t egal_pos = option.find('=');
    if (egal_pos == std::string::npos || egal_pos > 2)
    {
        MediaInfoNameSpace::String Option(ZenLib::Ztring().From_UTF8(option), 2, egal_pos-2);
        MediaInfoNameSpace::String Value;
        if (egal_pos != std::string::npos)
            Value.assign(Ztring().From_UTF8(option), egal_pos + 1, std::string::npos);
        else
            Value=__T('1');

        String Result = core->Menu_Option_Preferences_Option(Option, Value);
        if (!Result.empty())
        {
            report = Ztring(Result).To_UTF8();
            return -1;
        }
    }
    Options.push_back(option);
    return 0;
}

//---------------------------------------------------------------------------
bool MediaConchLib::ReportAndFormatCombination_IsValid(const std::vector<std::string>& files,
                                                       const std::bitset<MediaConchLib::report_Max>& reports,
                                                       const std::string& display, MediaConchLib::format& Format,
                                                       std::string& reason)
{
    // Forcing some formats
    if (Format == MediaConchLib::format_Text && !display.empty())
        Format = format_Xml; //Forcing Text (default) to XML

    if (Format != MediaConchLib::format_Xml && !display.empty())
    {
        reason = "If a display is used, no other output format can be used";
        return false;
    }

    if (files.size() > 1 && Format == MediaConchLib::format_Xml)
        Format = format_MaXml;

    if (reports.count() > 1 && Format == MediaConchLib::format_Xml)
        Format = MediaConchLib::format_MaXml;
    
    // Test of incompatibilities
    if (reports[MediaConchLib::report_MediaConch] && reports[MediaConchLib::report_MediaTrace]
        && Format == MediaConchLib::format_Text)
    {
        reason = "Combination of MediaConch and MediaTrace is currently not possible with Text output";
        return false;
    }

    return true;
}

//***************************************************************************
// Analyze
//***************************************************************************

//---------------------------------------------------------------------------
int MediaConchLib::analyze(const std::vector<std::string>& files)
{
    if (!files.size())
        return -1;

    bool registered = false;
    for (size_t i = 0; i < files.size(); ++i)
        analyze(files[i], registered);
    return 0;
}

//---------------------------------------------------------------------------
int MediaConchLib::analyze(const std::string& file, bool& registered)
{
    if (!file.length())
        return -1;

    if (use_daemon)
        return daemon_client->analyze(file, registered);
    return core->open_file(file, registered);
}

//---------------------------------------------------------------------------
bool MediaConchLib::is_done(const std::vector<std::string>& files, double& percent)
{
    if (!files.size())
        return true;

    bool done = true;
    percent = 0.0;
    double unit_percent = (1.0 / files.size()) * 100.0;
    for (size_t i = 0; i < files.size(); ++i)
    {
        double percent_done;
        if (is_done(files[i], percent_done))
            percent += unit_percent;
        else
        {
            percent += unit_percent * percent_done;
            done = false;
        }
    }
    return done;
}

//---------------------------------------------------------------------------
bool MediaConchLib::is_done(const std::string& file, double& percent)
{
    if (!file.length())
        return false;

    if (use_daemon)
        return daemon_client->is_done(file, percent);
    return core->is_done(file, percent);
}

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
int MediaConchLib::get_report(const std::bitset<report_Max>& report_set, format f,
                              const std::vector<std::string>& files,
                              const std::vector<std::string>& policies_names,
                              const std::vector<std::string>& policies_contents,
                              MediaConchLib::ReportRes* result,
                              const std::string* display_name,
                              const std::string* display_content)
{
    if (!files.size())
        return -1;

    if (use_daemon)
        return daemon_client->get_report(report_set, f, files,
                                         policies_names, policies_contents,
                                         result,
                                         display_name, display_content);
    return core->get_report(report_set, f, files,
                            policies_names, policies_contents,
                            result,
                            display_name, display_content);
}

//---------------------------------------------------------------------------
int MediaConchLib::remove_report(const std::vector<std::string>& files)
{
    if (!files.size())
        return -1;

    return core->remove_report(files);
}

//***************************************************************************
// Policy
//***************************************************************************

//---------------------------------------------------------------------------
bool MediaConchLib::validate_policy(const std::string& file, int policy,
                                    MediaConchLib::ReportRes* result,
                                    const std::string* display_name,
                                    const std::string* display_content)
{
    Policy* p = get_policy((size_t)policy);
    if (!p)
    {
        result->report = "Policy not found";
        return false;
    }

    if (use_daemon)
    {
        std::string policy_content;
        p->dump_schema(policy_content);
        return daemon_client->validate_policy(file, policy_content, result,
                                              display_name, display_content);
    }
    return core->validate_policy(file, policy, result, display_name, display_content);
}

//---------------------------------------------------------------------------
bool MediaConchLib::validate_policy_memory(const std::string& file, const std::string& policy,
                                           MediaConchLib::ReportRes* result,
                                           const std::string* display_name,
                                           const std::string* display_content)
{
    return core->validate_policy_memory(file, policy, result, display_name, display_content);
}

//---------------------------------------------------------------------------
int MediaConchLib::validate_policies(const std::string& file, const std::vector<std::string>& policies,
                                     MediaConchLib::ReportRes* result,
                                     const std::string* display_name,
                                     const std::string* display_content)
{
    if (!policies.size())
        return -1;

    std::string report;
    for (size_t i = 0; i < policies.size(); ++i)
    {
        if (core->validate_policy_file(file, policies[i], result, display_name, display_content) < 0)
            return -1;
        result->report += report;
        result->report += "\r\n";
    }

    return 0;
}

//***************************************************************************
// XSL Transformation
//***************************************************************************

//---------------------------------------------------------------------------
int MediaConchLib::transform_with_xslt_file(const std::string& report, const std::string& file, std::string& result)
{
    return core->transform_with_xslt_file(report, file, result);
}

//---------------------------------------------------------------------------
int MediaConchLib::transform_with_xslt_memory(const std::string& report, const std::string& memory, std::string& result)
{
    return core->transform_with_xslt_memory(report, memory, result);
}

//***************************************************************************
// Database
//***************************************************************************

//---------------------------------------------------------------------------
void MediaConchLib::load_configuration()
{
    core->load_configuration();
}

//---------------------------------------------------------------------------
void MediaConchLib::set_configuration_file(const std::string& file)
{
    core->set_configuration_file(file);
}

//---------------------------------------------------------------------------
const std::string& MediaConchLib::get_configuration_file() const
{
    return core->get_configuration_file();
}
//***************************************************************************
// Policy
//***************************************************************************

//---------------------------------------------------------------------------
void MediaConchLib::save_policies()
{
    for (size_t i = 0; i < core->policies.policies.size(); ++i)
        if (!core->policies.policies[i]->saved)
            core->policies.export_schema(NULL, i);
}

//---------------------------------------------------------------------------
void MediaConchLib::save_policy(size_t pos, const std::string* filename)
{
    if (pos > core->policies.policies.size())
        return;
    const char* path = NULL;
    if (filename)
        path = filename->c_str();
    core->policies.export_schema(path, pos);
}

//---------------------------------------------------------------------------
bool MediaConchLib::is_policies_saved() const
{
    for (size_t i = 0; i < core->policies.policies.size(); ++i)
        if (!core->policies.policies[i]->saved)
            return false;
    return true;
}

//---------------------------------------------------------------------------
bool MediaConchLib::is_policy_saved(size_t pos) const
{
    if (pos > core->policies.policies.size())
        return false;
    return core->policies.policies[pos]->saved;
}

//---------------------------------------------------------------------------
int MediaConchLib::import_policy_from_file(const std::string& filename, std::string& err)
{
    int ret = core->policies.import_schema(filename);
    if (ret < 0)
        err = core->policies.get_error();
    return ret;
}

//---------------------------------------------------------------------------
int MediaConchLib::import_policy_from_memory(const std::string& memory, const std::string& filename, std::string& err)
{
    int ret = core->policies.import_schema_from_memory(filename, memory.c_str(), memory.length());
    if (ret < 0)
        err = core->policies.get_error();
    return ret;
}

//---------------------------------------------------------------------------
bool MediaConchLib::policy_exists(const std::string& title)
{
    return core->policies.policy_exists(title);
}

//---------------------------------------------------------------------------
size_t MediaConchLib::get_policies_count() const
{
    return core->policies.policies.size();
}

//---------------------------------------------------------------------------
Policy* MediaConchLib::get_policy(size_t pos)
{
    if (pos > core->policies.policies.size())
        return NULL;
    return core->policies.policies[pos];
}

//---------------------------------------------------------------------------
const std::vector<Policy *>& MediaConchLib::get_policies() const
{
    return core->policies.policies;
}

//---------------------------------------------------------------------------
void MediaConchLib::add_policy(Policy* p)
{
    core->policies.policies.push_back(p);
}

//---------------------------------------------------------------------------
void MediaConchLib::remove_policy(size_t pos)
{
    core->policies.erase_policy(pos);
}

//---------------------------------------------------------------------------
void MediaConchLib::clear_policies()
{
    for (size_t i = 0; i < core->policies.policies.size(); ++i)
        delete core->policies.policies[i];
    core->policies.policies.clear();
}

//---------------------------------------------------------------------------
void MediaConchLib::set_use_daemon(bool use)
{
    if (!use)
        force_no_daemon = true;
    else
        force_no_daemon = false;
    use_daemon = use;
}

//---------------------------------------------------------------------------
bool MediaConchLib::get_use_daemon() const
{
    return use_daemon;
}

//---------------------------------------------------------------------------
void MediaConchLib::get_daemon_address(std::string& addr, int& port) const
{
    core->get_daemon_address(addr, port);
}

}