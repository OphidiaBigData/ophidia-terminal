/*
    Ophidia Terminal
    Copyright (C) 2012-2017 CMCC Foundation

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __OPH_WORKFLOW_DEFINE_H
#define __OPH_WORKFLOW_DEFINE_H

#define OPH_WORKFLOW_MIN_STRING		24
#define OPH_WORKFLOW_BASIC_SIZE		1024
#define OPH_WORKFLOW_MAX_STRING		2048
#define OPH_WORKFLOW_DOT_MAX_LEN	1048576
#define OPH_WORKFLOW_RANK_SIZE		128
#define OPH_WORKFLOW_ROOT_FOLDER	"/"

#define OPH_WORKFLOW_YES		"yes"
#define OPH_WORKFLOW_NO			"no"
#define OPH_WORKFLOW_NOP		"nop"
#define OPH_WORKFLOW_SKIP		"skip"
#define OPH_WORKFLOW_CONTINUE		"continue"
#define OPH_WORKFLOW_BREAK		"break"
#define OPH_WORKFLOW_REPEAT		"repeat"
#define OPH_WORKFLOW_DELETE		"oph_delete"

#define OPH_WORKFLOW_CLASSIC		"classic"
#define OPH_WORKFLOW_COMPACT		"compact"

#define OPH_OPERATOR_FOR		"oph_for"
#define OPH_OPERATOR_ENDFOR		"oph_endfor"
#define OPH_OPERATOR_IF			"oph_if"
#define OPH_OPERATOR_ELSEIF		"oph_elseif"
#define OPH_OPERATOR_ELSE		"oph_else"
#define OPH_OPERATOR_ENDIF		"oph_endif"

#endif				//__OPH_WORKFLOW_DEFINE_H
