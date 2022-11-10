#ifndef CALCULATION_ADDITIONAL_OUTPUTS_RATIONAL_LIST_CONTROLLER_H
#define CALCULATION_ADDITIONAL_OUTPUTS_RATIONAL_LIST_CONTROLLER_H

#include "expressions_list_controller.h"

namespace Calculation {

class RationalListController : public ExpressionsListController {
public:
  RationalListController(EditExpressionController * editExpressionController) :
    //ExpressionsListController(editExpressionController) {}
    ExpressionsListController(editExpressionController), m_exponent(0) {}

  void setExpression(Poincare::Expression e) override;
  // A FAIRE bool handleEvent(Ion::Events::Event event) override;

private:
  I18n::Message messageAtIndex(int index) override;
  int textAtIndex(char * buffer, size_t bufferSize, int index) override;
  void refreshScientificDisplay();
  static constexpr int k_indexOfFactorExpression = 2; // max 3 expressions
  int m_exponent;
};

}

#endif


