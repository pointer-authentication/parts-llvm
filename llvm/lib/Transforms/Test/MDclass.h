
using namespace llvm;
class MDclass{
public: 
        MDclass();
        void setPtrType(Type* ty);
        Type* getPtrType();
        void setFPtrType(bool fty);
        bool getFPtrType();
private:
        bool fnPtrType;
        Type* ptrType;
};

MDclass::MDclass(){
fnPtrType=false;
}

void MDclass::setPtrType(Type* ty){
ptrType=ty;
}

Type* MDclass::getPtrType(){
return ptrType;
}

void MDclass::setFPtrType(bool fty){
fnPtrType=fty;
}

bool MDclass::getFPtrType(){
return fnPtrType;
}

