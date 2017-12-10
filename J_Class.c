/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine class functions
  \  | | | | |    * 09-jun-2004: started new version
                  * 21-feb-2005: added exception sections/attribute
				                 parsing in jParseMethods function
*/

#include "J_Global.h"
#include "J_Memory.h"
#include "J_Native.h"


/**
 * Try to get class field by name and description
 */
J_Error_t jGetField(J_Class* jc,J_ClassFields** field,J_UtfString name,J_UtfString desc){
  J_UtfString fname, fdesc;
  uint16_t i;

  for(i=0;i<jc->fieldsCount;i++){
	*field = &(jc->fields[i]);
	fname = jGetUtfString(jc,(*field)->nameIdx );
	fdesc = jGetUtfString(jc,(*field)->descrIdx );
	if(name.len==fname.len && desc.len==fdesc.len &&
	   !strncmp(name.str,fname.str,name.len) &&
	   !strncmp(desc.str,fdesc.str,desc.len))
	  return EOK;
  }
  return EFIELDNOTFOUND;
}


/**
 * Try to get class field by index
 */
J_Error_t jGetFieldByIndex(J_Class* jc,J_ClassFields** field,uint16_t idx,J_Thread* thContext){
  J_Class* targetClass;
  J_UtfString fname,fdesc;
  uint16_t    classIdx, nameTypeIdx, nIdx, tIdx;

  uint8_t* p = jc->consts[idx] + jc->classmem;
  classIdx = getUint16(p+1);
  if(jGetClassByIndex(jc,&targetClass,classIdx,thContext)!=EOK || targetClass==NULL)
	return EFIELDNOTFOUND;
  
  nameTypeIdx = getUint16(p+3);
  nIdx = getUint16(jc->consts[nameTypeIdx]+jc->classmem+1); 
  tIdx = getUint16(jc->consts[nameTypeIdx]+jc->classmem+3);

  fname = jGetUtfString(jc,nIdx);
  fdesc = jGetUtfString(jc,tIdx);

  return jGetField(targetClass,field,fname,fdesc);
}


/**
 * Load constant from pool
 * @return tag - constant type
 */
uint8_t* jLoadClassConst(J_Class* jc,uint16_t idx,uint8_t* p,uint16_t* tag){
  switch((*tag=getUint8(p))){
  case CONSTANT_Utf8:
	p++;
	p += getUint16(p);
	p+=2;
	break;

  case CONSTANT_Integer:
  case CONSTANT_Float:
  case CONSTANT_Fieldref:
  case CONSTANT_Methodref:
  case CONSTANT_InterfaceMethodref: 
  case CONSTANT_NameAndType: 
	p++;
	p += 4;
	break;

  case CONSTANT_Class: 
  case CONSTANT_String: 
	p++;
	p += 2; 
	break;

  case CONSTANT_Long: 
  case CONSTANT_Double: 
	p++;
	p += 8; 
	break;
  }

#ifdef DEBUG_CONSTS 
  viewConst(jc,idx);
#endif
  
  return p;
}


/**
 * Parse constant pool from the class file 
 */
J_Error_t jParseConstants(J_Class *jc,uint8_t* p,uint32_t* poolLen){
  uint32_t size, i;
  uint8_t* start = p;
  uint16_t tag;
  
  *poolLen = 0;
  p += 8;
  jc->numConsts = getUint16(p);
  
  p += 2;
  if(jc->numConsts!=0){
	size = jc->numConsts * sizeof(uint32_t);

   /* allocate memory for constants pool */
	jc->consts = (uint32_t*) calloc(1,size);

	if(jc->consts==NULL){
	  dbg(DBG_MEMORY,"not enought memory for constant pool");
      return ENOMEM;
	}
	
   /* for each constant in the pool */
	for(i=1;i<jc->numConsts;i++){
	  if(p - jc->classmem>0x7fffffff){
		dbg(DBG_MEMORY,"class too large");
		return ENOMEM;
	  }

      jc->consts[i] = p - jc->classmem;
      p = jLoadClassConst(jc,i,p,&tag);

      if((tag==CONSTANT_Long)||(tag==CONSTANT_Double)) i++;
     }
    }
  *poolLen = p - start;
  return EOK;
}


/**
 *  Parse fields from the class file 
 */
J_Error_t jParseFields(J_Class* jc,uint8_t* p,uint32_t* poolLen){
  J_UtfString us;
  uint8_t* start = p;
  uint16_t i, j, attrCount, nameIdx, accessFlags;
  uint32_t bytesCount;
  
  *poolLen = 0;
  if(jc->fieldsCount>0){
	jc->fields = (J_ClassFields*) calloc(jc->fieldsCount,sizeof(J_ClassFields));
	if(jc->fields==NULL){
      dbg(DBG_MEMORY,"not enought memory for fields");
      return ENOMEM;
	}

   /* clear number of vars */
	jc->numVars = 0;

   /* start count cycle */
	for(i=0;i<jc->fieldsCount;i++){
	 /* get access flags */
      accessFlags = getUint16(p); p += 2;
      jc->fields[i].accessFlags = accessFlags;
	  
	 /* Check for static flags */
      if(!(accessFlags & ACCESS_STATIC)){
        jc->fields[i].var.varOffs = jc->numVars++;   /* local variable */
        accessFlags = 1;
      } else {  
		jc->fields[i].var.varOffs = 0; /* static var */
		accessFlags = 0;  
	  }

     /* get name idx and description idx */
      jc->fields[i].nameIdx = getUint16(p);	 p += 2; 
	  jc->fields[i].descrIdx = getUint16(p); p += 2; 

	  attrCount = getUint16(p); p += 2;
	  
	 /* for all attributes  */
	  for(j=0;j<attrCount;j++){
		nameIdx = getUint16(p);
		us = jGetUtfString(jc,nameIdx); p += 2;

		bytesCount = getUint32(p); p += 4;

	   // if final ... (constant)
		jc->fields[i].isFinal = 0;
		if(accessFlags && (us.len==13) && (bytesCount==2) &&
	       !strncmp(us.str,"ConstantValue",13)){
		  jc->fields[i].var = getConstant(jc,getUint16(p));
		  jc->fields[i].isFinal = 1;
		}
		p += bytesCount;
      }  // for(j= ...
	} // for(i= ...
  } // if(jc->fieldsCount>0)
  
  *poolLen = p - start;
  return EOK;
}


/**
 * Parse interfaces from the class file 
 */
J_Error_t jParseInterfaces(J_Class* jc,uint8_t* p,uint32_t* poolLen){
  uint16_t i;
  uint8_t* start = p;
  *poolLen = 0;

  if(jc->interfacesCount>0){
	jc->interfaces = (uint16_t*) calloc(jc->interfacesCount,sizeof(uint16_t));
	if(jc->interfaces==NULL){
	  dbg(DBG_MEMORY,"not enought memory for interfaces");
	  return ENOMEM;
	}
	for(i=0;i<jc->interfacesCount;i++){
      jc->interfaces[i] = getUint16(p);
      p += 2;
	}
  }
  *poolLen = p - start;
  return EOK;
}


/**
 *  Parse methods from the class file 
 */
J_Error_t jParseMethods(J_Class* jc,uint8_t* p,uint32_t* poolLen){
  uint16_t acc;
  int32_t  i,sz;
  uint8_t* start = p;
  J_UtfString str;
  
  *poolLen = 0;
  if(jc->methodsCount>0){
   /* allocate memory for methods */
	jc->methods = (J_ClassMethods*) calloc(jc->methodsCount,sizeof(J_ClassMethods));
	if(jc->methods==NULL) {
	  dbg(DBG_MEMORY,"not enought memory for methods");
	  return ENOMEM;
	}
	
	for(i=0;i<jc->methodsCount;i++){
	 /* get access flags */
	  acc = jc->methods[i].accessFlags = getUint16(p);
      p += 2;

      jc->methods[i].nameIdx = getUint16(p);
	  p += 2;
dbg(DBG_TEMPORARY," -------( '%s' )-------\n",jGetUtfString(jc,jc->methods[i].nameIdx).str);

	 /* Count method parameters */
      jc->methods[i].descrIdx = getUint16(p); 
      sz = countParams(jGetUtfString(jc,jc->methods[i].descrIdx));
      if(sz<0){
		jc->methods[i].paramsCount = 0;
	  } else jc->methods[i].paramsCount = sz;
      p += 2;

	 /* Is it method init ? */
      jc->methods[i].initMethod = 0;
      str = jGetUtfString(jc,jc->methods[i].nameIdx);
	  if((str.str[0]=='<') && (str.str[1]=='i')){
dbg(DBG_TEMPORARY,"method %s is <init>",str.str);
		jc->methods[i].initMethod = 1;
	  }

	 /* Is it return value (not void) ? */
      str = jGetUtfString(jc,jc->methods[i].descrIdx);
	  jc->methods[i].isRetValue = 0;
      if(str.str[str.len-1]!='V')
		jc->methods[i].isRetValue = 1;

     /* how many attribures have its method */
      jc->methods[i].attributesCount = getUint16(p);
      p += 2;

     /* allocate memory for the attributes */
      jc->methods[i].methodsAttr = (J_MethodsAttr*) calloc(jc->methods[i].attributesCount,
														   sizeof(J_MethodsAttr));
      if(jc->methods[i].methodsAttr==NULL){
		dbg(DBG_MEMORY,"not enought memory for methods attrib structure");
		return ENOMEM;
      }

     /* get method attributes */
	 /* If the method is either native or abstract, its method_info 
	  * structure must not have a Code attribute. 
	  */

	 /* Here need to load native functions */
	  if(!(acc & (ACCESS_NATIVE|ACCESS_ABSTRACT))){
		J_MethodsExceptions* ea;
		int j, k;
		
	   /* Check all attributes */
		for(j=0;j<jc->methods[i].attributesCount;j++) {
		  jc->methods[i].methodsAttr[j].attributeNameIdx = getUint16(p);
		  p += 2;

		 /* Is it 'Exceptions' attribute ? /added 21-feb-2005/ */
		  ea = &(jc->methods[i].exceptionsAttr);
		  if(!strncmp(jGetUtfString(jc,jc->methods[i].methodsAttr[j].attributeNameIdx).str,"Exceptions",10)) {
			ea->len = getUint32(p);
			p+=4;
			ea->numberOfExceptions = getUint16(p);
			p+=2;
			ea->exceptionsTable = (uint16_t*) p;
			
/* dbg(DBG_EXCEPTIONS,"Exception attribute: len = %d, number = %d",ea->len,ea->numberOfExceptions); */
/*  {int k; */
/* 			for(k=0;k<ea->numberOfExceptions;k++){ */
/* 			  uint16_t ei = getUint16(p); */
/* 			  p+=2; */
/* dbg(DBG_EXCEPTIONS," (%s)",jGetUtfString(jc,getUint16(jc->consts[ei]+jc->classmem+1)).str); */
/* 			} */
/*  } */
			
		   /* skip exceptions table */
			p += 2 * ea->numberOfExceptions;
			continue;
		  } else {
			ea->numberOfExceptions = 0;
			ea->len = 0;
		  }

		  jc->methods[i].methodsAttr[j].len = getUint32(p);
		  p += 4;

		 /* Get max stack and max locals */
		  jc->methods[i].maxStack = getUint16(p);
		  p += 2;
		
		  jc->methods[i].maxLocals = getUint16(p);
		  p += 2;

		 /* Get code section */
		  jc->methods[i].codeLength = getUint32(p);
		  p += 4;
		  if(jc->methods[i].codeLength>0){
			jc->methods[i].code = p;
		  } else {
			dbg(DBG_TEMPORARY,"No code section enabled in ");
		  }
		  p += jc->methods[i].codeLength;

		 /* Get exception table section */
		  jc->methods[i].excTableLength = getUint16(p);
/* dbg(DBG_EXCEPTIONS,"exceptions table len = %d",jc->methods[i].excTableLength); */
		  p += 2;
		  
/* if(jc->methods[i].excTableLength>0) */
/*   printMemory(p,8 * jc->methods[i].excTableLength); */
 
		  if(jc->methods[i].excTableLength>0) jc->methods[i].excTable = p;
		  p += 8 * jc->methods[i].excTableLength;

		 /* Skip code attributes */
		  sz = getUint16(p); // attributes_count
		  p += 2;
		  for(k=0;k<sz;k++){
			uint16_t t;
			p += 6; // attr_name_index(2) + attr_length(4)
			t = getUint16(p); // line_numb_table_len
			p += 2;
			p += t*4;         // line_table
		  }
		} // for(j=0...
	  }   // if(!NATIVE...
	  
dbg(DBG_TEMPORARY,"maxLocals = %d, maxStack = %d, paramsCount = %d",jc->methods[i].maxLocals,
	jc->methods[i].maxStack,jc->methods[i].paramsCount);
	  
	 /* If method is native load it */
	  if(acc & ACCESS_NATIVE)
		jLoadNativeFunction(jc,i);
	  
	} // for(i=0...
  } // if(jc->methodsCount ... 

 /* And General Attributes section skip ! */
  *poolLen = p - start;

  return EOK;
}


/**
 * Get class by index
 * pointer to found class return in the '*c' variable
 */
J_Error_t jGetClassByIndex(J_Class* jc,J_Class** c,uint16_t idx,J_Thread* thContext){
  uint8_t* p = jc->consts[idx] + jc->classmem + 1;
  uint16_t clsIdx;
  
 /* Get class index */
  clsIdx = getUint16(p);

 /* Try to load it class and return */
  return jGetClass(c,jGetUtfString(jc,clsIdx),thContext);
}


/**
 *  Function for loading class file by given name
 */
J_Error_t jGetClass(J_Class** c,J_UtfString name,J_Thread* currentThread){
  J_Object  obj;
  J_ClassMethods* method = NULL;
  uint8_t*  p;
  uint32_t  idx,len;
  J_Error_t r;

 /* Lookup in the hash table */
  idx = jUtfHash(name) % MAX_CLASS_HASH;
  (*c) = globalHash[idx];
  while(*c){
	J_UtfString tmp = jGetClassName((*c),(*c)->thisClassIdx);
	if(tmp.len==name.len && !strncmp(name.str,tmp.str,name.len)) break;
	(*c) = (*c)->nextClass;
  }
  if(*c) return EOK;
  
 /* try to allocate memory */
  (*c) = (J_Class*) calloc(1,sizeof(J_Class));
  if(!(*c)){
	dbg(DBG_MEMORY,"error allocating memory for class");
	return ENOMEM;
  }

 /* try allocate object */
  if((obj=allocateObject((*c),1))<0){
	dbg(DBG_MEMORY,"error allocate object");
	return ENOMEM;
  }
  
 /* backward link */
  (*c)->obj = obj;

 /* load class */
  p = (uint8_t*) (loadFile(name,&len)).pc;
  if(p==NULL || len==0)
	return ECLASSNOTFOUND;

 /* clear superclass pointer */
  (*c)->superClass = 0;

 /* where is allocated into memory */
  (*c)->classmem = p;

 /* parse constants */
  if(jParseConstants((*c),p,&len)!=EOK)
	return ECONSTSFAIL;
  p += len;

 /* get access_flags section */
  (*c)->accessFlags = (uint16_t*) p;
  p += 2;

 /* get this class index */
  (*c)->thisClassIdx = getUint16(p);
  p += 2;

 /* add to hash table */
  (*c)->nextClass = globalHash[idx];
  globalHash[idx] = (*c);

 /* get super_class index */
  (*c)->numberSuperClasses = 0;
  (*c)->superClassIdx = getUint16(p);
  p += 2;
  
 /* get interfaces count and parse */
  (*c)->interfacesCount = getUint16(p); 
  p += 2; 
  len = 0;
  if((*c)->interfacesCount>0)
	if(jParseInterfaces((*c),p,&len)!=EOK)
	  return EINTERFACEFAIL;
  if(len) p += len;

 /* get fields count and parse */
  (*c)->fieldsCount = getUint16(p); 
  p += 2; 
  len = 0;
  if((*c)->fieldsCount>0)
	if(jParseFields((*c),p,&len)!=EOK)
	  return EFIELDSFAIL;
  if(len) p += len;

 /* get methods count and parse */
  (*c)->methodsCount = getUint16(p); 
  p += 2;
  len = 0;
  if(jParseMethods((*c),p,&len)!=EOK)
	return EMETHODSFAIL;
  p += len;

 /* try to load superclasses */
  if((*c)->superClassIdx>0){
	r = jGetClassByIndex((*c),&((*c)->superClass),(*c)->superClassIdx,currentThread);
	if(r!=EOK) return r;
	else {
	  (*c)->numberSuperClasses++;
	}
  }	

 /* find class init <clinit> method : static{...} */
  r = jGetMethod((*c),jUtfCreate("<clinit>"),jUtfCreate("()V"),NULL,&method);
 	
 /* and try to execute it */
  if(r==EOK && method!=NULL){
	jExecMethod((*c),method,NULL,0,currentThread);
  }
  
  return EOK;
}

