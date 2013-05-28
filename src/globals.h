#include "Headers.h"
#include "Files.h"
#include "CmdArgs.h"
#include "Scanner.h"
#include "DFA.h"
#include "OrdSet.h"
#include "BitStore.h"
#include "SArray.h"
#include "HashTable.h"
#include "Vars.h"
#include "Model.h"
#include "Formula.h"
#include "Forest.h"
#include "CTLCheck.h"
#include "Buchi.h"
#include "LTLCheck.h"

enum {
	TK_WS,
	TK_MODELOP,
	TK_MODELCL,
	TK_INITIALSTATE,
	TK_COMPARE,
	TK_COMPAREMID,

	TK_IMPLIES,
	TK_NEGATION,
	TK_AND,
	TK_OR,
	TK_PROPVAR,
	TK_INT,
	TK_AU,
	TK_EU,
	TK_UCL,
	TK_PAROP,
	TK_PARCL,
	TK_U,
	TK_R,
	TK_W,
	TK_TRUE,
	TK_BOTTOM,
	// these tokens must be in sequence:
	TK_X, 
	TK_F,
	TK_G,
	TK_AG,
	TK_EG,
	TK_AF,
	TK_EF,
	TK_AX,
	TK_EX,
	// end sequence
	TK_TOTAL,
};

