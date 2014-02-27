#include "ObjectSymbolTable.h"
#include <algorithm>
#include "CompilationExceptions.h"

ObjectSymbolTable::ObjectSymbolTable() {
	analyzer.reference = this;
}

ObjectSymbolTable::~ObjectSymbolTable() {
	for(map<string, PropertySymbolTable*>::iterator it = classes.begin(); it != classes.end(); ++it) {
		delete it->second;
	}
}

boost::optional<SemanticError*> ObjectSymbolTable::addClass(string name) {
	addingclass_name = name;

	if(classes.count(addingclass_name)) {
		return boost::optional<SemanticError*>(new SemanticError(MULTIPLE_CLASS_DEFINITION));
	}

	addingclass_symbol = new PropertySymbolTable(&analyzer);
	addingclass_symbol->classname = name;
	addingclass_hassubclass = false;

	classes[addingclass_name] = addingclass_symbol;

	return boost::optional<SemanticError*>();
}

boost::optional<SemanticError*> ObjectSymbolTable::addInheritance(string childname, bool as_subclass) {
	if(addingclass_hassubclass && as_subclass) {
		return boost::optional<SemanticError*>(new SemanticError(MORE_THAN_ONE_SUBCLASS));
	}

	if(childname == addingclass_name) {
		return boost::optional<SemanticError*>(new SemanticError(SELF_INHERITANCE));
	}

	if(analyzer.isASubtypeOfB(childname, addingclass_name)) {
		return boost::optional<SemanticError*>(new SemanticError(CIRCULAR_INHERITANCE));
	}

	if(addingclass_symbol->parentage.count(childname)) {
		return boost::optional<SemanticError*>(new SemanticError(MULTIPLE_INHERITANCE));
	}

	addingclass_hassubclass = addingclass_symbol->parentage[childname] = as_subclass;

	return boost::optional<SemanticError*>();
}

void ObjectSymbolTable::propagateInheritance() {
	map<string, pair<PropertySymbolTable*, bool> > passed;
	for(map<string, PropertySymbolTable*>::iterator it = classes.begin(); it != classes.end(); ++it) {
		inheritances_gathered[it->first] = pair<PropertySymbolTable*, bool>(it->second, false);
	}

	for(map<string, pair<PropertySymbolTable*, bool> >::iterator it = inheritances_gathered.begin(); it != inheritances_gathered.end(); ++it) {
		propagateInheritanceToParent(it->first);
	}

	//for(map<string, pair<PropertySymbolTable*, bool> >::iterator it = inheritances_gathered.begin(); it != inheritances_gathered.end(); ++it) {
		//delete it->second;
	//}
}

void ObjectSymbolTable::propagateInheritanceToParent(string childname) {
	pair<PropertySymbolTable*, bool>* current = &inheritances_gathered.find(childname)->second;
	if(current->second) return; // Already propagated

	for(map<string, bool>::iterator it = current->first->parentage.begin(); it != current->first->parentage.end(); ++it) {
		propagateInheritanceToParent(it->first);
		propagateInheritanceTables(current->first, find(it->first), it->second);
	}

	current->second = true;
}

PropertySymbolTable* ObjectSymbolTable::find(string name) {
	std::map<string, PropertySymbolTable*>::iterator searcher = classes.find(name);
	if(!classes.count(name)) {
		SymbolNotFoundException* error = new SymbolNotFoundException();
		error->errormsg = "Could not find symbol ";
		error->errormsg += name;
		error->errormsg += " in object table";
		throw error;
	}

	return searcher->second;
}

void ObjectSymbolTable::assertTypeIsValid(Type* type) {
	if(type->type == TYPE_CLASS) {
		if(classes.count(type->typedata._class.classname)) return;

		throw new SemanticError(CLASSNAME_NOT_FOUND, type->typedata._class.classname + string(" is not a valid type"));
	} else {
		if(type->typedata.lambda.returntype != NULL)
			assertTypeIsValid(type->typedata.lambda.returntype);
		if(type->typedata.lambda.arguments == NULL) return;

		int i;
		for(i = 0; i < type->typedata.lambda.arguments->typecount; i++)
			assertTypeIsValid(type->typedata.lambda.arguments->types[i]);
	}
}

void ObjectSymbolTable::printEntryPoints(EntryPointAnalyzer* entryanalyzer) {
	for(map<string, PropertySymbolTable*>::iterator it = classes.begin(); it != classes.end(); ++it) {
		if(!entryanalyzer->checkClassNeedsCanBeMain(it->second->getNeeds())) continue;
		entryanalyzer->printClass(it->first);
		it->second->printEntryPoints(entryanalyzer);
	}
}

TypeAnalyzer* ObjectSymbolTable::getAnalyzer() {
	return &analyzer;
}
