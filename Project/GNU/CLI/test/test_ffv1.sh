#!/bin/sh

PATH_SCRIPT=$(dirname "$0")
. "$PATH_SCRIPT/utils.sh"

FILES_DIRECTORY="$PATH_SCRIPT/SampleFiles/PolicyTestFiles/FFV1/"
FILES="`ls "$FILES_DIRECTORY"`"

for i in $FILES
do

    DATA="`./mediaconch \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_invalid
    output_has_not_mc_xml
    output_has_not_mi_xml
    output_has_not_mt_xml

    DATA="`./mediaconch -mc -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_mc_xml
    output_has_not_mi_xml
    output_has_not_mt_xml

    DATA="`./mediaconch -mi -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_not_mc_xml
    output_has_mi_xml
    output_has_not_mt_xml

    DATA="`./mediaconch -mt -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_not_mc_xml
    output_has_not_mi_xml
    output_has_mt_xml

    DATA="`./mediaconch -mt -mi -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_not_mc_xml
    output_has_mi_xml
    output_has_mt_xml

    DATA="`./mediaconch -mc -mi -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_mc_xml
    output_has_mi_xml
    output_has_not_mt_xml

    DATA="`./mediaconch -mc -mt -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_mc_xml
    output_has_not_mi_xml
    output_has_mt_xml

    DATA="`./mediaconch -mt -mi -mc -fx \"--implementationschema=$RESOURCE_PATH/schema/MatroskaSchema.xml\" \"$FILES_DIRECTORY/$i\"`"
    cmd_is_ok
    xml_is_correct
    output_has_mc_xml
    output_has_mi_xml
    output_has_mt_xml

done
