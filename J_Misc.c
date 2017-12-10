/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine miscelaneouse functions
  \  | | | | |    * 16-jun-2004: started
                  
*/

#include "J_Global.h"
#include "J_Memory.h"

J_Object createJavaStringObjectFromUtfString(J_UtfString s);

/**
 *  Debug output function 
 *  14-jun-2004:
 */
void dbg(uint16_t dbglev,const char *fmt,...){
  int n=0;
#ifdef DEBUG_JVM
  va_list args;
  va_start(args,fmt);
  if(!(dbglev & j_dbg_filter)) 
	return;
  while(dbglev>>=1) n++;
#ifdef JIMP_OS
  vmsg("[%d] ",n);
  vmsg(fmt,args);
  vcrlf();
#else
  printf("[%d] ",n);
  vprintf(fmt,args);
  printf("\n");
#endif
  va_end(args);
#endif
}


/**
 *  Error function 
 *  16-jun-2004:
 */
void _err(int errlevel,const char *fmt,...){
  va_list args;
  char *m = (errlevel==ERR_FATAL) ? "FATAL> " : "WARNING> ";
  va_start(args,fmt);
#ifdef JIMP_OS
  vmsg(m);
  vmsg(fmt,args);
  if(errlevel==ERR_FATAL) while(1);
#else
  printf(m);
  vprintf(fmt,args);
  if(errlevel==ERR_FATAL) exit(1);
#endif
}


uint32_t hash(register uint8_t *k){
  return hash0(k,strlen(k),HASH0);
}

uint32_t hash0(k,length,initval)
	 register uint8_t *k;        /* the key */
	 register uint32_t  length;  /* the length of the key */
	 register uint32_t  initval; /* the previous hash, or an arbitrary value */
{
  register uint32_t a,b,c,len;

 /* Set up the internal state */
  len = length;
  a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
  c = initval;         /* the previous hash value */

 /*---------------------------------------- handle most of the key */
  while (len >= 12){
	a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
	b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
	c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));
	mix(a,b,c);
	k += 12; len -= 12;
  }

 /*------------------------------------- handle the last 11 bytes */
  c += length;
  switch(len){              /* all the case statements fall through */
    case 11: c+=((uint32_t)k[10]<<24);
    case 10: c+=((uint32_t)k[9]<<16);
    case 9 : c+=((uint32_t)k[8]<<8);
   /* the first byte of c is reserved for the length */
    case 8 : b+=((uint32_t)k[7]<<24);
    case 7 : b+=((uint32_t)k[6]<<16);
    case 6 : b+=((uint32_t)k[5]<<8);
    case 5 : b+=k[4];
    case 4 : a+=((uint32_t)k[3]<<24);
    case 3 : a+=((uint32_t)k[2]<<16);
    case 2 : a+=((uint32_t)k[1]<<8);
    case 1 : a+=k[0];
   /* case 0: nothing left to add */
  }
  mix(a,b,c);
  return c;
}


/**
 *  Replace all 'c' symbols on the 'r'
 */
char* strcrepl(char* cname,char c,char r){
  int l = strlen(cname);
  char* p = (char*) malloc(l);
  while(l>=0){
	if(cname[l]==c) p[l] = r;
	else p[l] = cname[l];
	l--;
  }
  return p;
}

/**
 *  Create and return UTF string
 */
J_UtfString jUtfCreate(char* plainString){
  J_UtfString string;
  string.str = plainString;
  string.len = strlen(plainString);
  return string;
}

/**
 *  Create and return UTF string
 *  text data will dublicated
 */
J_UtfString* jUtfCreateDup(char* plainString){
  J_UtfString *string = (J_UtfString*) malloc(sizeof(J_UtfString));
  string->str = strdup(plainString);
  string->len = strlen(plainString);
  return string;
}

/**
 *  Free utf string memory
 */
void jUtfDelete(J_UtfString* string){
  if(string->str) free(string->str);
  free(string);
}


/**
 *  Free utf resources
 */
void jUtfFree(J_UtfString string){
  if(string.str) free(string.str);
  string.len = 0;
}


/**
 *  Get hash code for UTF string
 */
uint32_t jUtfHash(J_UtfString string){
  return hash0(string.str,string.len,HASH0);
}

/**
 *  Print utf string
 */
void jPrintUtf(J_UtfString* string){
  int i;
  for(i=0;i<string->len;i++)
	printf("%c",string->str[i]);
}


/**
 *  Get uint32 from ptr
 */
uint32_t getUint32(uint8_t *b){
  return (uint32_t)((uint32_t)((b)[0])<<24|
					(uint32_t)((b)[1])<<16|(uint32_t)((b)[2])<<8|(uint32_t)((b)[3]));
}

/**
 *  Get int32 from ptr
 */
int32_t getInt32(uint8_t *b){
  return (int32_t)((uint32_t)((b)[0])<<24|
				   (uint32_t)((b)[1])<<16|(uint32_t)((b)[2])<<8|(uint32_t)((b)[3]));
}

/**
 *  Get uint16 from ptr
 */
uint16_t getUint16(uint8_t *b){
  return (uint16_t)(((b)[0]<<8)|(b)[1]);
}

/**
 *  Get int16 from ptr
 */
int16_t getInt16(uint8_t *b){
  return (int16_t)(((b)[0]<<8)|(b)[1]);
}

/**
 *  Get uint8 from ptr
 */
uint8_t getUint8(uint8_t *b){
  return (uint8_t)(b)[0];
}

/**
 *  Get float32 from ptr
 */
float getFloat32(uint8_t *buf){
  union { uint32_t i; float f; } m;

 /* we need to make sure we're aligned before casting */
  m.i = ((uint32_t)buf[0] << 24)|
	((uint32_t)buf[1] << 16)|
	((uint32_t)buf[2] << 8 )|
	(uint32_t)buf[3];
  return m.f;
}

/**
 *  Get float64 from ptr
 */
double getFloat64(uint8_t *buf){
  union { uint64_t i; double d; } m;

 /* we need to make sure we're aligned before casting */
  m.i = ((uint64_t)buf[0] << 56 ) |
	((uint64_t)buf[1] << 48 ) |
	((uint64_t)buf[2] << 40 ) |
	((uint64_t)buf[3] << 32 ) |
	((uint64_t)buf[4] << 24 ) |
	((uint64_t)buf[5] << 16 ) |
	((uint64_t)buf[6] << 8  ) |
	((uint64_t)buf[7] << 0  );
  return m.d;
}


/**
 *  Count parameters in description
 */
int32_t countParams(J_UtfString desc){
  uint32_t n = 0;
  char*    c;

  c = desc.str;
  if(*c++!='(') return -1;
  while(1){
    switch(*c){
	case 'B':
	case 'C':
	case 'F':
	case 'I':
	case 'S':
	case 'Z':
	  n++;
	  c++;
	  break;

	case 'L':
	  c++;
	  while(*c++!=';');
	  n++;
	  break;

	case '[':
	  c++;
	  break;

	case ')':
	  return n;
	  
	case 'D': /* longs/doubles not supported */
	case 'J':
	default:
	  return -1;
	}
  }
  return -1;
}

/** Get UTF string from class by index
 */
J_UtfString jGetUtfString(J_Class* jc,uint16_t idx){
  J_UtfString utf;
  utf.str = (char*)(jc->classmem+jc->consts[idx]+1);
  utf.len = getUint16(utf.str);
  utf.str += 2;
  return utf;
}


/**
 * Get classname from class by index
 */
J_UtfString jGetClassName(J_Class* jc,uint16_t idx){
  char *p = (char*)(jc->classmem+jc->consts[idx]+1);
  return jGetUtfString(jc,getUint16(p));
}


/**
 * Get constant value from class by index
 */
J_Var getConstant(J_Class *jc,uint16_t idx){
  J_Var v;
  int string_idx;
  uint8_t* p = jc->consts[idx] + jc->classmem;
  
  switch(getUint8(p)){
  case CONSTANT_Integer:
	v.vInt = getUint32(p+1);	/* bug was here: get16 instead get32 ! */
	break;
	
  case CONSTANT_Float:
	v.vFloat = getFloat32(p+1);
	break;

  case CONSTANT_String:
	string_idx = getUint16(p+1);
	v.obj = createJavaStringObjectFromUtfString(jGetUtfString(jc,string_idx));
/* 	  create_UTF_from_string(get_utf_string(jc,string_idx)); */

/* Example accessing to the string 
JIMP_obj k;
char   *c;
 k = JIMP_STRING_ARRAY_OBJ( v.obj );
 c = (char*)JIMP_ARRAY_START(k);
 printf("~~~~~~~~~~~~~");
 for(string_idx=0;string_idx<20;string_idx+=2) printf("%c", *(c+string_idx) );
 */
	break;

  case CONSTANT_Long:
  case CONSTANT_Double:
  default:
	v.obj = 0; /* bad constant */
	break;
  }
  return v;
}


/**  
 *  Get method in class by signature
 */
J_Error_t jGetMethod(J_Class* jc,J_UtfString name,J_UtfString desc,
					 J_Class** vc,J_ClassMethods** method){
  J_UtfString mname, mdesc;
  uint32_t    i, n;
  
 /* Initially null */
  *method = NULL;
  n = jc->numberSuperClasses;

/* printf("\tSearch: %s%s\n",name.str,desc.str); */
 
  while(1){
	for(i=0;i<jc->methodsCount;i++){
	  *method = &jc->methods[i];
	  mname = jGetUtfString(jc,(*method)->nameIdx);
	  mdesc = jGetUtfString(jc,(*method)->descrIdx);
//printf("\t\tcheck: %s%s <-> %s%s\n",mname.str,mdesc.str,name.str,desc.str);
	  if((name.len==mname.len) && (desc.len==mdesc.len) &&
		 !strncmp(name.str,mname.str,name.len) &&
		 !strncmp(desc.str,mdesc.str,desc.len)){
		if(vc){
		 //printf("*vc=%x\n",*vc);
		  *vc = jc;
/* CLASS_INFO((*vc)); printf("---> FOUND <---\n"); */
		}
	   return EOK;
	  }
	}

   /* not a virtual lookup or no superclass */
/* 	if(!vc || (n==0)) break; */
	if(!vc || !jc->numberSuperClasses) break;
	n--;

   /* lookup in superclass */
	if(jc->superClass) jc = jc->superClass;
	
/* { */
/*  J_UtfString tmp = jGetClassName(jc,jc->thisClassIdx);  */
/*  printf("Seek %s%s in superclass: %s !\n",name.str,desc.str,tmp.str); */
/* } */
	
  }

  return EMETHODNOTFOUND;
}


/**
 * Create java/lang/String class object from utf string
 */
J_Object createJavaStringObjectFromUtfString(J_UtfString s){
  J_Object obj, charArrayObj;
  uint16_t* charStart;
  uint32_t  i;
  J_Class*  sClass;
  J_Error_t e;
	
 /* create and fill char array */
  charArrayObj = createArray(5,s.len); /* type "C" - chars */
  if(!charArrayObj) return 0;

 /* push object on extended stack for avoid GC */
  jExtPushObj(charArrayObj);
 
  charStart = (uint16_t*) J_ARRAY_START(charArrayObj);
  for(i=0;i<s.len;i++)
	charStart[i] = (uint16_t) s.str[i];

 /* create String object and set char array */
  if((e=jGetClass(&sClass,jUtfCreate("java/lang/String"),NULL))!=EOK)
	err(ERR_FATAL,"Can't load class 'java/lang/String' : %d",e);
 
  obj = allocateObject(sClass,1);

  if(obj!=0){
	J_STRING_ARRAY_OBJ(obj) = charArrayObj;
   /* experimental */
	J_ARRAY_LEN(charArrayObj) = s.len;
  }

 /* pop object from extended stack */
  jExtPopObj();
  
  return obj;
}


/**
 * Create java/lang/String class object from chars array
 */
J_Object createString(char* buf){
  return createJavaStringObjectFromUtfString(jUtfCreate(buf));
}


/**
 *  View constant's value
 */
void viewConst(J_Class* jc,uint16_t idx){
  char  tmp[256];
  uint8_t* p = jc->consts[idx] + jc->classmem;
  uint64_t _l;

  switch(getUint8(p)){
  case CONSTANT_Utf8:
	p++;
	p+=2;
	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,p,getUint16(p-2));
	dbg(DBG_TEMPORARY,"(%d)UTF> %s",idx,tmp);
	p += getUint16(p-2);
	break;
  case CONSTANT_Integer:
	p++;
	dbg(DBG_TEMPORARY,"(%d)Integer>(%d)",idx,getInt32(p));
	break;
  case CONSTANT_Float:
	p++;
	dbg(DBG_TEMPORARY,"(%d)Float>(%f)",idx,getFloat32( p ));
	break;
  case CONSTANT_Fieldref:
	p++;
	dbg(DBG_TEMPORARY,"(%d)Fieldref> %d",idx,getInt16(p));
	break;
  case CONSTANT_Methodref:
	p++;
	dbg(DBG_TEMPORARY,"(%d)Methodref> %d",idx,getInt16(p));
	break;
  case CONSTANT_InterfaceMethodref: 
	p++;
	dbg(DBG_TEMPORARY,"(%d)InterfaceMethodref> %d",idx,getInt16(p));
	break;
  case CONSTANT_NameAndType: 
	p++;
	break;
  case CONSTANT_Class: 
	p++;
	dbg(DBG_TEMPORARY,"(%d)Class> %d",idx,getInt16(p));
	break;
  case CONSTANT_String: 
	p++;
	dbg(DBG_TEMPORARY,"(%d)String> %d",idx,getInt16(p));
	break;
  case CONSTANT_Long: 
	p++;
	_l = (uint64_t)(*p);
	dbg(DBG_TEMPORARY,"(%d)String>%ld\n",idx,_l);
	break;
  case CONSTANT_Double: 
	p++;
	dbg(DBG_TEMPORARY,"(%d)Double>%f\n",idx,getFloat64(p));
	break;
  }
}


/**
 * Check is classes source and target are compatible ?
 */ 
int compatible(J_Class* source,J_Class* target,J_Thread* jth){
  int targetInterface;
  uint32_t i, n;

  if(!source || !target)
	return 0; // source or target is array

  targetInterface = 0;
  if((getUint16((uint8_t*)target->accessFlags) & ACCESS_INTERFACE))
	targetInterface = 1;
  
  n = source->numberSuperClasses;
  while (1){
	if(targetInterface){ 
	  for(i=0;i<getUint16(((uint8_t*) source->accessFlags + 6));i++){
		uint16_t clsIdx;
		J_Class* iClass;

		clsIdx = getUint16(((uint8_t*) source->accessFlags + 8+(i*2)));
		if(jGetClassByIndex(source,&iClass,clsIdx,jth)!=EOK){
		  err(ERR_FATAL,"[compatible] Can't get class by index");
		}
		
	   // NOTE: Either one of the interfaces in the source class can
	   // equal the target interface or one of the interfaces
	   // in the target interface (class) can equal one of the
	   // interfaces in the source class for the two to be compatible
		if(iClass==target) return 1;
		if(compatible(iClass,target,jth)) return 1;
	  }
	} else if(source==target) return 1;
	if(n==0) break;
   // look in superclass
	source = &source->superClass[--n];
  }
  return 0;
}


/**
 *  Print memory dump in hex view
 */
void printMemory(uint8_t* memory,int n){
  int i;
  for(i=0;i<n;i++)
#ifdef JIMP_OS
	kprintf("0x%02x ",memory[i]);
  vcrlf();
#else
	printf("0x%02x ",memory[i]);
  printf("\n");
#endif
}
