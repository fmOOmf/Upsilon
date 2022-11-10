#include "integer_list_controller.h"
#include <poincare/based_integer.h>
#include <poincare/integer.h>
#include <poincare/empty_layout.h>
#include <poincare/factor.h>
#include "../app.h"
#include "../../shared/poincare_helpers.h"
#include "poincare/decimal.h"
#include "apps/apps_container.h"
#include "poincare/layout_helper.h"

using namespace Poincare;
using namespace Shared;

namespace Calculation {



  // => Formats existants
  Integer::Base baseAtIndex(int index) {
    switch (index) {
      case 0:
        return Integer::Base::Decimal;
      case 1:
        return Integer::Base::Hexadecimal;
      default:
        assert(index == 2);
        return Integer::Base::Binary;
    }
  }

  // **************************************
  //dl11 code
  /*
  void IntegerListController::setExpression(Poincare::Expression e) {
    ExpressionsListController::setExpression(e);
    static_assert(k_maxNumberOfRows >= k_indexOfFactorExpression + 1, "k_maxNumberOfRows must be greater than k_indexOfFactorExpression");
    assert(!m_expression.isUninitialized() && m_expression.type() == ExpressionNode::Type::BasedInteger);

    // Number to display
    Integer integer = static_cast<BasedInteger &>(m_expression).integer();
    
    // m_layouts initialisation : decimal, hexa et binaire
    for (int index = 0; index < k_indexOfFactorExpression - 1; ++index) {
      m_layouts[index] = integer.createLayout(baseAtIndex(index));
    }
    
    // => ??
    const int * value = (int *)integer.digits();
    int shift = GlobalPreferences::sharedGlobalPreferences()->getDecimalShift();
    // => Number exponent
    m_exponent = (int)log10(abs(*value));

    Decimal decimal = Decimal::Builder(integer, m_exponent);

    // => buffer =le nombre initial sous forme de char  numberOfChars=nb caracteres nombre en X.YYYx10E
    char buffer[Poincare::DecimalNode::k_maxBufferSize];
    int numberOfChars = decimal.serialize(buffer, Poincare::DecimalNode::k_maxBufferSize, Preferences::PrintFloatMode::Scientific);

    // If a dot exist in char buffer => shift can be performed
    if (*(UTF8Helper::CodePointSearch(buffer, Ion::InternalStorage::k_dotChar)) != 0) {
      int originalNumber = numberOfChars;
      int maxShift = originalNumber - 3 - (int)log10(abs(m_exponent)) - 1 - 2;

      if (shift > maxShift) {
        shift = maxShift;
        GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(maxShift);
      }
      
      //test
      // ====================================
      Decimal decimal2 = Decimal::Builder(integer, m_exponent-shift);
      char buffer2[Poincare::DecimalNode::k_maxBufferSize];
      int numberOfChars2 = decimal2.serialize(buffer2, Poincare::DecimalNode::k_maxBufferSize, Preferences::PrintFloatMode::Scientific);

      Decimal decimal3 = Decimal::Builder(integer, m_exponent-10);
      char buffer3[Poincare::DecimalNode::k_maxBufferSize];
      int numberOfChars3 = decimal3.serialize(buffer3, Poincare::DecimalNode::k_maxBufferSize, Preferences::PrintFloatMode::Scientific);      
      //
      // ======================================


      // => ??
      decimal = Decimal::Builder(integer, m_exponent-shift);
      numberOfChars = decimal.serialize(buffer, Poincare::DecimalNode::k_maxBufferSize, Preferences::PrintFloatMode::Scientific);

      // Decalage du dot
      for (int i = 0; i < shift; i++) {
        char c = buffer[i+1];
        buffer[i+1] = buffer[i+2];
        buffer[i+2] = c;
      }

      // if E00 => Do not display Exponent (maintain ?)
      if (m_exponent - shift == 0) {
        buffer[numberOfChars-1] = '\0';
      }

      if (shift == maxShift && m_exponent - shift != 0) {
        strlcpy(&buffer[shift + 1], &buffer[shift + 2], strlen(&buffer[shift + 1]));
      }
    } // fin decalage

    // => Fill in scientific
    Layout layout = LayoutHelper::String(buffer, numberOfChars);
    m_layouts[k_indexOfFactorExpression - 1] = layout;

    
    // Computing Prime factorExpression
    Expression factor = Factor::Builder(m_expression.clone());
    PoincareHelpers::Simplify(&factor, App::app()->localContext(), ExpressionNode::ReductionTarget::User);

    // => fill in prime only if defined (k_indexOfFactorExpression 0 ... n-1)
    if (!factor.isUndefined()) {
      m_layouts[k_indexOfFactorExpression] = PoincareHelpers::CreateLayout(factor);
    }
  } // fin void expression_list_controller

  // => Labels des infos complementaires
  // specific dl11
  I18n::Message IntegerListController::messageAtIndex(int index) {
    switch (index) {
      case 0:
        return I18n::Message::DecimalBase;
      case 1:
        return I18n::Message::HexadecimalBase;
      case 2:
        return I18n::Message::BinaryBase;
      case 3:
          return I18n::Message::Scientific;
      default:
        return I18n::Message::PrimeFactors;
    }
  }


  // => Def HandleEvent
  bool IntegerListController::handleEvent(Ion::Events::Event event) {
    if (m_listController.selectableTableView()->selectedRow() == 3) {
      int decimalShift = GlobalPreferences::sharedGlobalPreferences()->getDecimalShift();
      if (event == Ion::Events::Left) {
        if (decimalShift > 0 && Preferences::sharedPreferences()->displayMode() != Preferences::PrintFloatMode::Engineering) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift - 1);
          refreshScientificDisplay();
        } else if (decimalShift > 3 && Preferences::sharedPreferences()->displayMode() == Preferences::PrintFloatMode::Engineering) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift - 3);
          refreshScientificDisplay();
        }
        return true;
        } else if (event == Ion::Events::Right) {
        if (Preferences::sharedPreferences()->displayMode() == Preferences::PrintFloatMode::Engineering && m_exponent - decimalShift - 3 >= 0) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift + 3);
        } else if (m_exponent - decimalShift - 1 >= 0) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift + 1);
        }
        refreshScientificDisplay();
        return true;
      }
    }
    return ListController::handleEvent(event);
  }

  // => Refresh affichage
  void IntegerListController::refreshScientificDisplay() {
    Expression e = m_expression;
    Container::activeApp()->dismissModalViewController();
    setExpression(e);
    Container::activeApp()->displayModalViewController(this, 0.f, 0.f, Metric::CommonTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
    m_listController.selectableTableView()->selectCellAtLocation(0, 3); // dl11 => 3
  }
  // fin code dl11
  // **************************************
  */

  // **************************************
  // fmOOmf code
  //
  void IntegerListController::setExpression(Poincare::Expression e) {
    ExpressionsListController::setExpression(e);
    static_assert(k_maxNumberOfRows >= k_indexOfFactorExpression + 1, "k_maxNumberOfRows must be greater than k_indexOfFactorExpression");
    assert(!m_expression.isUninitialized() && m_expression.type() == ExpressionNode::Type::BasedInteger);

    // Number to display
    Integer integer = static_cast<BasedInteger &>(m_expression).integer();
    
    // Le nombre au format entier
    const int * value = (int *)integer.digits();
    int shift = GlobalPreferences::sharedGlobalPreferences()->getDecimalShift();
    // Number exponent
    m_exponent = (int)log10(abs(*value));

    Decimal decimal = Decimal::Builder(integer, m_exponent);

  // => buffer =le nombre initial sous forme de char  numberOfChars=nb caracteres nombre en X.YYYx10E
    char buffer[Poincare::DecimalNode::k_maxBufferSize];
    int numberOfChars = decimal.serialize(buffer, Poincare::DecimalNode::k_maxBufferSize, Preferences::PrintFloatMode::Scientific);

    // If a dot exist in char buffer => shift can be performed
    if (*(UTF8Helper::CodePointSearch(buffer, Ion::InternalStorage::k_dotChar)) != 0) {

      //max +9 or -9 shifts
      // int maxShift = 9; // A FINALISER
      int maxShift = numberOfChars - 3 - (int)log10(abs(m_exponent)) - 1 - 2;
      if (shift > maxShift) {
        shift = maxShift;
        //GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(maxShift);
        GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(maxShift);
      }
      if (shift < -maxShift) {
        shift = -maxShift;
        //GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(maxShift);
        GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(-maxShift);
      }

      // Mise a jour avec le nouvel exposant
      decimal = Decimal::Builder(integer, m_exponent-shift);
      numberOfChars = decimal.serialize(buffer, Poincare::DecimalNode::k_maxBufferSize, Preferences::PrintFloatMode::Scientific);


      // =========================================     
      // TEST
      // =========================================
      /*
      int pos_dot = *(UTF8Helper::CodePointSearch(buffer, Ion::InternalStorage::k_dotChar)); // dot position
      int pos_10p  = *(UTF8Helper::CodePointSearch(buffer, 341)); // Position debut 10 puissance (3 caracteres) : 341 264 207
      int pos_null = *(UTF8Helper::CodePointSearch(buffer, 0)); // Position fin chaine
      bool tmp_neg = false; // inutile
      int nb_mant = pos_10p-pos_dot; // nb chiffres significatifs
      int nb_tot = pos_null+1; // longueur chaine initiale
      const char * char_zero = "000000000000000000000000"; // !! test syntaxe (a supprimer)
      char buffer_out[Poincare::DecimalNode::k_maxBufferSize]; // Initialisation

      char tmp_mantisse[nb_mant]; // les chiffres significatifs sans le point 
      tmp_mantisse[0]= buffer[pos_dot-1];
      strlcpy(&tmp_mantisse[1], &buffer[pos_dot+1], nb_mant);

      
      if (shift > 0) { // Cas decalage positif
        if (shift == nb_mant-1) { // aucun chiffres à rajouter apres decalage => pas de point
          strncat(&buffer_out,&tmp_mantisse, shift+1); // shift+1 chiffres 
        }
        if (shift < nb_mant-1) { // il restera des chiffres à rajouter => il faut un point + chiffres restants
          strncat(&buffer_out,&tmp_mantisse, shift+1); // shift+1 chiffres 
          strncat(&buffer_out,&tmp_mantisse[pos_dot],1); // ajout du point
          strncat(&buffer_out, &tmp_mantisse[shift+1], nb_mant-(shift+1) ); // mantnb -(shift+1) 
        }
        if (shift > nb_mant-1) { // pas assez de chiffres  => completer par des zeros
          strncat(&buffer_out,&tmp_mantisse, nb_mant); // =nb_mant-1 decalage 
          strncat(&buffer_out, &char_zero, shift-(nb_mant-1) ); // mantnb -(shift+1) 
        }
        // Ajout Exposant a faire
      } else if (shift < 0) { // Cas decalage negatif
        // decalage
        // Ajout Exposant
      }

      // A faire : suppression exposant ?
      // A Faire : suppression point final
      // A faire : signe
      // =========================================
      // TEST
      // =========================================
      */

      // Decalage du dot
      for (int i = 0; i < shift; i++) {
        char c = buffer[i+1];
        buffer[i+1] = buffer[i+2];
        buffer[i+2] = c;
      }

      // if E00 => Do not display Exponent (maintain ?)
      if (m_exponent - shift == 0) {
        buffer[numberOfChars-1] = '\0';
      }

      // Enlever le point final si pas de nombre après. Strlen= 1 de plus pour copier le \0 terminal
      if (shift == maxShift && m_exponent - shift != 0) {
        strlcpy(&buffer[shift + 1], &buffer[shift + 2], strlen(&buffer[shift + 1]));
      }
    } // fin decalage

    // => Creation Layout Scientific  
    Layout layoutsci = LayoutHelper::String(buffer, numberOfChars);
    

    // Computing Prime factorExpression
    Expression factor = Factor::Builder(m_expression.clone());
    PoincareHelpers::Simplify(&factor, App::app()->localContext(), ExpressionNode::ReductionTarget::User);


    // Layout fill in
    m_layouts[0] = integer.createLayout(baseAtIndex(0)); // decimal
    m_layouts[1] = layoutsci; // scientific
    m_layouts[2] = integer.createLayout(baseAtIndex(1)); // hexadecimal
    m_layouts[3] = integer.createLayout(baseAtIndex(2)); // binary

    // => fill in prime only if defined (k_indexOfFactorExpression 0 ... n-1)
    if (!factor.isUndefined()) {
      // fmOOmf
      m_layouts[4] = PoincareHelpers::CreateLayout(factor);
    }

  } // fin void expression_list_controller


  //
  // => Labels des infos complementaires
  // specific fmOOmf
  I18n::Message IntegerListController::messageAtIndex(int index) {
    switch (index) {
      case 0:
        return I18n::Message::DecimalBase;
      case 1:
        return I18n::Message::Scientific;
      case 2:
        return I18n::Message::HexadecimalBase;
      case 3:
        return I18n::Message::BinaryBase;
      case 4:
        return I18n::Message::PrimeFactors;
      default:
        return I18n::Message::DecimalBase;
    }
  }
  // => Def HandleEvent
  bool IntegerListController::handleEvent(Ion::Events::Event event) {
    if (m_listController.selectableTableView()->selectedRow() == 1) {
      int decimalShift = GlobalPreferences::sharedGlobalPreferences()->getDecimalShift();
      if (event == Ion::Events::Left) {
        if (decimalShift > 0 && Preferences::sharedPreferences()->displayMode() != Preferences::PrintFloatMode::Engineering) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift - 1);
          refreshScientificDisplay();
        } else if (decimalShift > 3 && Preferences::sharedPreferences()->displayMode() == Preferences::PrintFloatMode::Engineering) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift - 3);
          refreshScientificDisplay();
        }
        return true;
      } else if (event == Ion::Events::Right) {
        if (Preferences::sharedPreferences()->displayMode() == Preferences::PrintFloatMode::Engineering && m_exponent - decimalShift - 3 >= 0) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift + 3);
        } else if (m_exponent - decimalShift - 1 >= 0) {
          GlobalPreferences::sharedGlobalPreferences()->setDecimalShift(decimalShift + 1);
        }
        refreshScientificDisplay();
        return true;
      }
    }
    return ListController::handleEvent(event);
  }

  // => Refresh affichage
  void IntegerListController::refreshScientificDisplay() {
    Expression e = m_expression;
    Container::activeApp()->dismissModalViewController();
    setExpression(e);
    Container::activeApp()->displayModalViewController(this, 0.f, 0.f, Metric::CommonTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
    //m_listController.selectableTableView()->selectCellAtLocation(0, 3);
    m_listController.selectableTableView()->selectCellAtLocation(0, 1);
  }
  // FIN CODE fmOOmf
  // ****************************
  //


} // end namespace Calculation