/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2018 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/
#include "policy.h"

Policy::Policy(void) : TableObject()
{
	obj_type=OBJ_POLICY;
	permissive = false;
	policy_cmd = PolicyCmdType::all;

	attributes[ParsersAttributes::PERMISSIVE] = QString();
	attributes[ParsersAttributes::COMMAND] = QString();
	attributes[ParsersAttributes::USING_EXP] = QString();
	attributes[ParsersAttributes::CHECK_EXP] = QString();
	attributes[ParsersAttributes::ROLES] = QString();
}

void Policy::setParentTable(BaseTable *table)
{
	if(table && table->getObjectType() != OBJ_TABLE)
	{
		throw Exception(Exception::getErrorMessage(ERR_ASG_INV_OBJECT_TYPE)
						.arg(this->obj_name)
						.arg(this->getTypeName())
						.arg(BaseObject::getTypeName(OBJ_TABLE)),
						ERR_ASG_INV_OBJECT_TYPE,__PRETTY_FUNCTION__,__FILE__,__LINE__);
	}

	TableObject::setParentTable(table);
}

void Policy::setPermissive(bool value)
{
	setCodeInvalidated(permissive != value);
	permissive = value;
}

bool Policy::isPermissive(void)
{
	return(permissive);
}

void Policy::setPolicyCommand(PolicyCmdType cmd)
{
	setCodeInvalidated(policy_cmd != cmd);
	policy_cmd = cmd;
}

PolicyCmdType Policy::getPolicyCommand(void)
{
	return(policy_cmd);
}

void Policy::setUsingExpression(const QString &expr)
{
	setCodeInvalidated(using_expr != expr);
	using_expr = expr;
}

QString Policy::getUsingExpression(void)
{
	return(using_expr);
}

void Policy::setCheckExpression(const QString &expr)
{
	setCodeInvalidated(check_expr != expr);
	check_expr = expr;
}

QString Policy::getCheckExpression(void)
{
	return(check_expr);
}

void Policy::addRole(Role *role)
{
	if(!role)
		throw Exception(ERR_ASG_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	if(std::find(roles.begin(), roles.end(), role) == roles.end())
		roles.push_back(role);
}

void Policy::removeRoles(void)
{
	roles.clear();
	setCodeInvalidated(true);
}

vector<Role *> Policy::getRoles(void)
{
	return(roles);
}

QString Policy::getCodeDefinition(unsigned def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return(code_def);

	QStringList rol_names;

	if(getParentTable())
		attributes[ParsersAttributes::TABLE]=getParentTable()->getName(true);

	attributes[ParsersAttributes::COMMAND] = ~policy_cmd;

	for(auto role : roles)
		rol_names.append(role->getName(true));

	attributes[ParsersAttributes::PERMISSIVE] = (permissive ? ParsersAttributes::_TRUE_ : QString());
	attributes[ParsersAttributes::USING_EXP] = using_expr;
	attributes[ParsersAttributes::CHECK_EXP] = check_expr;
	attributes[ParsersAttributes::ROLES] = rol_names.join(QString(", "));

	return(BaseObject::__getCodeDefinition(def_type));
}

QString Policy::getSignature(bool format)
{
	if(!getParentTable())
		return(BaseObject::getSignature(format));

	return(QString("%1 ON %2").arg(this->getName(format)).arg(getParentTable()->getSignature(true)));
}

QString Policy::getAlterDefinition(BaseObject *object)
{
	Policy *policy=dynamic_cast<Policy *>(object);

	if(!policy)
		throw Exception(ERR_OPR_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	try
	{
		QStringList rol_names, aux_rol_names;
		attribs_map attribs;

		attributes[ParsersAttributes::ALTER_CMDS]=BaseObject::getAlterDefinition(object);

		if(this->using_expr.simplified() != policy->using_expr.simplified())
			attribs[ParsersAttributes::USING_EXP] = policy->using_expr;

		if(this->check_expr.simplified() != policy->check_expr.simplified())
			attribs[ParsersAttributes::CHECK_EXP] = policy->check_expr;

		for(auto role : this->roles)
			rol_names.append(role->getName(true));

		for(auto role : policy->roles)
			aux_rol_names.append(role->getName(true));

		rol_names.sort();
		aux_rol_names.sort();

		if(!rol_names.isEmpty() && aux_rol_names.isEmpty())
			attribs[ParsersAttributes::ROLES] = ParsersAttributes::UNSET;
		else if(rol_names.join(QString(", ")) != aux_rol_names.join(QString(", ")))
			attribs[ParsersAttributes::ROLES] = aux_rol_names.join(QString(", "));

		copyAttributes(attribs);
		return(BaseObject::getAlterDefinition(this->getSchemaName(), attributes, false, true));
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
	}
}

bool Policy::isRoleExists(Role *role)
{
	if(!role)	return(false);
	return(std::find(roles.begin(), roles.end(), role) != roles.end());
}