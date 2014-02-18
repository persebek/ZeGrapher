/****************************************************************************
**  Copyright (c) 2013, Adel Kara Slimane, the ZeGrapher project <contact@zegrapher.com>
**
**  This file is part of the ZeGrapher project, version 2.0.
**
**  ZeGrapher is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU General Public License as published by the
**  Free Software Foundation, either version 3 of the License, or (at your
**  option) any later version.
**
**  This file is distributed in the hope that it will be useful, but
**  WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/


#include "Calculus/treecreator.h"

TreeCreator::TreeCreator(short callingObjectType)
{
    funcType = callingObjectType;

    refFunctions << "acos" << "asin" << "atan" << "cos" << "sin" << "tan" << "sqrt"
                 << "log" << "ln" << "abs" << "exp" << "floor" << "ceil" << "cosh"
                 << "sinh" << "tanh" << "E" << "e";
    functions << "f" << "g" << "h" << "p" << "r" << "m";
    antiderivatives << "F" << "G" << "H" << "P" << "R" << "M";
    derivatives << "f'" << "g'" << "h'" << "p'" << "r'" << "m'";
    sequences << "u" << "v" << "l" << "w" << "q" << "z";

    constants << "π" << "pi" << "Pi" << "PI";
    constantsVals << M_PI << M_PI << M_PI << M_PI ;

    vars << "x" << "t" << "n" << "k";
    authorizedVars << false << false << false << false;

    operators << '^' << '*' << '/' << '+' << '-';
    operatorsPriority << POW << OP_HIGH << OP_HIGH << OP_LOW << OP_LOW;
    operatorsTypes << POW << MULTIPLY << DIVIDE << PLUS << MINUS;

    refreshAuthorizedVars();
}

void TreeCreator::refreshAuthorizedVars()
{
    if(funcType == FUNCTION)
    {
        authorizedVars[0] = authorizedVars[3] = true; // x and k
    }
    else if(funcType == SEQUENCE)
    {
        authorizedVars[2] = authorizedVars[3] = true; // k and n
    }
    else if(funcType == PARAMETRIC_EQ)
    {
        authorizedVars[1] = authorizedVars[3] = true; // k and t
    }
    else if(funcType == DATA_TABLE_EXPR)
    {
        authorizedVars[0] = true; // only x, which is the old cell value.
    }
}

FastTree* TreeCreator::getTreeFromExpr(QString expr, bool &ok, QStringList additionnalVars)
{    
    FastTree *tree = NULL;

    customVars = additionnalVars;

    insertMultiplySigns(expr);
    ok = check(expr);

    if(ok)
        tree = createFastTree(decompTypes.size()-1, 0);

    return tree;
}

void TreeCreator::allow_k(bool state)
{
    authorizedVars[3] = state;
}

void TreeCreator::insertMultiplySigns(QString &formule)
{
    for(int i = 0 ; i < formule.size()-1; i++)
    {
        if((formule[i].isDigit() && formule[i+1].isLetter()) ||
                (formule[i].isLetter() && formule[i+1].isDigit()) ||
                (formule[i].isDigit() && formule[i+1] == '(') ||
                (formule[i] == ')' && formule[i+1] == '(') ||               
                (formule[i] == ')' && (formule[i+1].isDigit() || formule[i+1].isLetter())) ||
                (vars.contains(QString(formule[i])) && vars.contains(QString(formule[i+1]))) ||
                (i != 0 && !formule[i-1].isLetter() && vars.contains(QString(formule[i])) && formule[i+1] == '('))
        {
            formule.insert(i+1, QString("*"));
            i++;
        }
        else if(formule[i] == '-' && formule[i+1].isLetter())
        {
            formule.insert(i+1, QString("1*"));
            i++;
        }
    }
}

QList<int> TreeCreator::getCalledFuncs(QString expr)
{
    QList<int> calledFuncs;

    if(expr.isEmpty())
        return calledFuncs;

    bool doesContain = false;
    int i = 0, letterStart;

    QStringList calledObjects;

    while(i < expr.size())
    {
        if(expr[i].isLetter())
        {
            letterStart = i;

            while(expr[i].isLetter() && i < expr.size())
                i++;

            calledObjects << expr.mid(letterStart, i - letterStart);
        }
        i++;
    }

    for(i = 0 ; i < functions.size() ; i++)
    {
        doesContain = false;
        doesContain = doesContain || calledObjects.contains(antiderivatives[i]);
        doesContain = doesContain || calledObjects.contains(functions[i]);
        doesContain = doesContain || calledObjects.contains(derivatives[i]);

        if(doesContain && !calledFuncs.contains(i))
            calledFuncs << i;
    }

    return calledFuncs;
}

QList<int> TreeCreator::getCalledSeqs(QString expr)
{
    QList<int> calledSeqs;

    if(expr.isEmpty())
        return calledSeqs;

    int i = 0, letterStart;

    QStringList calledObjects;

    while(i < expr.size())
    {
        if(expr[i].isLetter())
        {
            letterStart = i;

            while(expr[i].isLetter() && i < expr.size())
                i++;

            calledObjects << expr.mid(letterStart, i - letterStart);
        }
        i++;
    }

    for(i = 0 ; i < sequences.size() ; i++)
    {
        if(calledObjects.contains(sequences[i]) && !calledSeqs.contains(i))
            calledSeqs << i;
    }

    return calledSeqs;
}

bool TreeCreator::check(QString formule)
{
    formule.remove(' ');
    formule.replace("²", "^2");
    decompPriorites.clear();
    decompTypes.clear();
    decompValeurs.clear();

    if(formule.isEmpty())
        return false;

    bool chiffre = true, ouvrepth = true, numberSign = true, varOrFunc = true, canEnd = false,
         operateur = false, fermepth = false;

    short pth = 0;

    for(int i = 0 ; i < formule.size(); i++)
    {
        if((formule[i].isDigit() && chiffre) || ((formule[i]=='-' || formule[i] == '+') && numberSign && i+1 < formule.size() && formule[i+1].isDigit()))
        {
            bool ok = false, dejavirgule = false;
            int numStart = i, numDigits = 1;
            i++;

            while(!ok && i < formule.size())
            {
                if(formule[i].isDigit())
                    i++;
                else if((formule[i]==',' || formule[i]=='.') && !dejavirgule)
                {
                    dejavirgule = true;
                    i++;
                }
                else ok = true;
            }

            numDigits = i - numStart;
            i--;

            QString nombre = formule.mid(numStart, numDigits);
            decompPriorites << NOMBRE;
            decompTypes << NOMBRE;
            decompValeurs << nombre.toDouble(&ok);
            if(!ok)
                return false;

            ouvrepth = varOrFunc = numberSign = false;
            chiffre = operateur = fermepth = canEnd = true;
        }
        else if(formule[i].isLetter() && varOrFunc)
        {
            int letterPosStart = i;

            while(i+1 < formule.size() && (formule[i+1].isLetter() || formule[i+1] == '_')) { i++ ; }

            if(i != formule.size() && formule[i+1] == '\'')
                i++;

            int numLetters = i - letterPosStart + 1;           

            QString name = formule.mid(letterPosStart, numLetters);

            if(refFunctions.contains(name) || antiderivatives.contains(name) || functions.contains(name) ||
                    derivatives.contains(name) || sequences.contains(name))
            {
                if(i+1 >= formule.size() || (formule[i+1] != '(' && formule[i] != 'e' && formule[i] != 'E'))
                    return false;

                decompPriorites << FONC;
                decompValeurs << 0.0;
                ouvrepth = true;
                chiffre = operateur = canEnd = fermepth = varOrFunc = numberSign = false;

                if(refFunctions.contains(name))
                {
                    decompTypes << refFunctions.indexOf(name) + REF_FUNC_START + 1;

                    if(name == "E" || name == "e")
                        chiffre = numberSign = ouvrepth = true;
                }

                else if(antiderivatives.contains(name) && funcType == FUNCTION)
                    decompTypes << antiderivatives.indexOf(name) + INTEGRATION_FUNC_START + 1;

                else if(functions.contains(name))
                    decompTypes << functions.indexOf(name) + FUNC_START + 1;

                else if(derivatives.contains(name))
                    decompTypes << derivatives.indexOf(name) + DERIV_START + 1;

                else if(sequences.contains(name) && funcType == SEQUENCE)
                    decompTypes << sequences.indexOf(name) + SEQUENCES_START + 1;

                else return false;
            }

            else if(constants.contains(name) || customVars.contains(name) || vars.contains(name))
            {
                varOrFunc = numberSign = false;
                ouvrepth = chiffre = operateur = fermepth = canEnd = true;

                if(vars.contains(name) && authorizedVars[vars.indexOf(name)])
                {
                    decompTypes << vars.indexOf(name) + VARS_START + 1;
                    decompPriorites << VAR;
                    decompValeurs << 0.0;
                }

                else if(constants.contains(name))
                {
                    decompPriorites << NOMBRE;
                    decompTypes << NOMBRE;
                    decompValeurs << constantsVals[constants.indexOf(name)];
                }

                else if(customVars.contains(name))
                {
                    decompTypes << ADDITIONNAL_VARS_START + customVars.indexOf(name);
                    decompPriorites << VAR;
                    decompValeurs << customVars.indexOf(name);
                }

                else return false;
            }

            else return false;

        }       
        else if(operators.contains(formule[i]) && operateur)
        {
            short pos = operators.indexOf(formule[i]);

            decompTypes << operatorsTypes[pos];
            decompPriorites << operatorsPriority[pos];
            decompValeurs << 0.0 ;

            ouvrepth = chiffre = varOrFunc = true;
            operateur = numberSign = fermepth = canEnd = false;
        }
        else if(formule[i]=='(' && ouvrepth)
        {           
            pth++;

            decompTypes << PTHO ;
            decompPriorites << PTHO;
            decompValeurs << 0.0 ;

            numberSign = chiffre = varOrFunc = ouvrepth = true;
            operateur = fermepth = canEnd = false;
        }
        else if(formule[i]==')' && fermepth && pth > 0)
        {            
            pth--;

            decompTypes << PTHF ;
            decompPriorites << PTHF ;
            decompValeurs << 0.0 ;

            operateur = fermepth = canEnd = true;
            chiffre = numberSign = ouvrepth = varOrFunc = false;

        }        
        else return false;
    }

    return pth == 0 && canEnd;
}

FastTree* TreeCreator::createFastTree(int debut, int fin)
{
    FastTree *racine = new FastTree;
    racine->right = NULL;
    racine->left = NULL;
    racine->value = NULL;

    short pths = 0, posPthFerme = 0, posPthOuvert = 0;
    bool debutPthFerme = false;

    if(debut == fin)
    {
        if(decompPriorites[debut] == NOMBRE)
        {
            racine->type = NOMBRE;
            racine->value = new double;
            *racine->value = decompValeurs[debut];
        }
        else racine->type = decompTypes[debut];

        return racine;
    }

    for(char op = 0; op < 5; op++)
    {
        for(short i = debut ; i >= fin ; i--)
        {
            if(decompPriorites[i] == PTHF)
            {
                if(!debutPthFerme)
                {
                    debutPthFerme = true;
                    posPthFerme = i - 1;
                }
                pths--;
            }
            else if(decompPriorites[i] == PTHO)
            {
                pths++;
                if(pths == 0)
                {
                    posPthOuvert = i + 1;
                    if(op == PTHO)
                    {
                        delete racine;
                        racine = createFastTree(posPthFerme, posPthOuvert);
                        return racine;
                    }
                }
            }
            else if(pths == 0 && decompPriorites[i] == op)
            {
                racine->type = decompTypes[i];
                racine->right = createFastTree(debut, i + 1);
                if(op != FONC)
                    racine->left = createFastTree(i - 1, fin);
                return racine;
            }
        }
    }
    return racine;
}

void TreeCreator::deleteFastTree(FastTree *tree)
{
    delete tree->value;
    if(tree->left != NULL)
        deleteFastTree(tree->left);
    if(tree->right != NULL)
        deleteFastTree(tree->right);
    delete tree;
}