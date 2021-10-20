// Copyright (c) Elucid Bioimaging

#include "targetDefine.h"

/********************************* processingParameters (part of the TARGET PACKAGE) **************************************************
 *
 * See processingParameters.h for description of the package purpose and contents.  This file has member functions for classes in the package.
 *
 **************************************************************************************************************************************/

/*!
  \class CompositionControlSlider
  \bold {Note:} CompositionControlSlider inherits QSlider for implementation specific reasons. Adjusting any single handle specific properties like 
  \list
  \o QAbstractSlider::sliderPosition
  \o QAbstractSlider::value
  \endlist
  has no effect. However, all slider specific properties like
  \list
  \o QAbstractSlider::minimum
  \o QAbstractSlider::maximum
  \o QAbstractSlider::orientation
  \o QAbstractSlider::pageStep
  \o QAbstractSlider::singleStep
  \o QSlider::tickInterval
  \o QSlider::tickPosition
  \endlist
  are taken into consideration.
 */
CompositionControlSlider::CompositionControlSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  offset = 0;
  position = 0;
  lastPressed = CompositionControlSlider::NoHandle;
  LRNC_lowerPressed = QStyle::SC_None;
  LRNC_upperPressed = QStyle::SC_None;
  CALC_lowerPressed = QStyle::SC_None;
  CALC_upperPressed = QStyle::SC_None;
  blockTracking = false;

  HUvalueLUT.clear();
  HUvalueLUT << -1018 << -982 << -947 << -913 << -881 << -850 << -820 << -792 << -764 << -737 << -712 << -687 << -664 << -641 << -619 << \
  -598 << -577 << -558 << -539 << -521 << -503 << -487 << -470 << -455 << -440 << -425 << -411 << -398 << -385 << -372 << -360 << -349 << -338 << -327 << \
  -316 << -306 << -297 << -287 << -278 << -270 << -261 << -253 << -245 << -238 << -230 << -223 << -217 << -210 << -204 << -197 << -192 << -186 << -180 << \
  -175 << -170 << -164 << -160 << -155 << -150 << -146 << -141 << -137 << -133 << -129 << -125 << -122 << -118 << -115 << -111 << -108 << -105 << -101 << \
  -98 << -95 << -93 << -90 << -87 << -84 << -82 << -79 << -77 << -74 << -72 << -69 << -67 << -65 << -63 << -61 << -59 << -57 << -55 << -53 << -51 << \
  -49 << -47 << -45 << -43 << -42 << -40 << -38 << -36 << -35 << -33 << -32 << -30 << -28 << -27 << -25 << -24 << -22 << -21 << -20 << -18 << -17 << \
  -15 << -1 << 0 << 2 << 3 << 4 << 5 << 6 << 8 << 9 << 10 << 11 << 12 << 14 << 15 << 16 << 17 << 19 << 20 << 21 << 23 << 24 << 25 << 26 << 28 << 29 << \
  30 << 32 << 33 << 35 << 36 << 37 << 39 << 40 << 42 << 43 << 44 << 46 << 47 << 49 << 50 << 52 << 53 << 55 << 57 << 58 << 60 << 61 << 63 << 65 << 66 << \
  68 << 70 << 71 << 73 << 75 << 77 << 79 << 80 << 82 << 84 << 86 << 88 << 90 << 92 << 94 << 96 << 98 << 100 << 102 << 105 << 107 << 109 << 111 << 114 << \
  116 << 118 << 121 << 123 << 126 << 128 << 131 << 134 << 136 << 139 << 142 << 145 << 148 << 151 << 154 << 157 << 160 << 163 << 166 << 169 << 173 << \
  176 << 180 << 183 << 187 << 191 << 194 << 198 << 202 << 206 << 211 << 215 << 219 << 223 << 228 << 233 << 237 << 242 << 247 << 252 << 257 << 262 << \
  268 << 273 << 279 << 285 << 291 << 297 << 303 << 309 << 316 << 322 << 329 << 336 << 343 << 351 << 358 << 366 << 374 << 382 << 390 << 398 << 407 << \
  416 << 425 << 434 << 444 << 454 << 464 << 474 << 485 << 496 << 507 << 518 << 530 << 542 << 555 << 567 << 580 << 594 << 607 << 621 << 636 << 651 << \
  666 << 682 << 698 << 714 << 731 << 748 << 766 << 784 << 803 << 822 << 842 << 863 << 884 << 905 << 927 << 950 << 973 << 997 << 1021 << 1047 << 1072 << \
  1099 << 1126 << 1154 << 1183 << 1213 << 1243 << 1275 << 1307 << 1340 << 1374 << 1409 << 1444 << 1481 << 1519 << 1558 << 1598 << 1639 << 1681 << 1725 << \
  1769 << 1815 << 1862 << 1910 << 1960 << 2011 << 2064 << 2118 << 2173 << 2230 << 2289 << 2349 << 2411 << 2475 << 2541 << 2608 << 2677 << 2748 << 2821 << \
  2897 << 2974 << 3053 << 3135 << 3219 << 3305 << 3394 << 3485 << 3579 << 3675 << 3775 << 3877 << 3981 << 4089 << 4200 << 4314 << 4431 << 4551 << 4675 << \
  4802 << 4933 << 5068;
  
  setMaximum(HUvalueLUT.size()-1);
  setTickPosition(QSlider::TicksBelow);
  setTickInterval(HUvalueLUT.size()/3);
}

void CompositionControlSlider::initStyleOption(QStyleOptionSlider* option, CompositionControlSlider::SpanHandle handle) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QSlider::initStyleOption(option);
  switch (handle) {
    case CompositionControlSlider::LRNC_lowerHandle:
      option->sliderPosition = LRNC_lowerPos;
      break;
    case CompositionControlSlider::LRNC_upperHandle:
      option->sliderPosition = LRNC_upperPos;
      break;
    case CompositionControlSlider::CALC_lowerHandle:
      option->sliderPosition = CALC_lowerPos;
      break;
    case CompositionControlSlider::CALC_upperHandle:
      option->sliderPosition = CALC_upperPos;
      break;
    case CompositionControlSlider::NoHandle:
      break;
  }
}

int CompositionControlSlider::pixelPosToRangeValue(int pos) const
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(pos) << std::endl;
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  const QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
  const QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  int sliderLength = sr.width();
  int sliderMin = gr.x();
  int sliderMax = gr.right() - sliderLength + 1;
  return QStyle::sliderValueFromPosition(minimum(), maximum(), pos - sliderMin, sliderMax - sliderMin, opt.upsideDown);
}

void CompositionControlSlider::drawSpan(QStylePainter* painter, const QRect& rect, QColor color) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  const QSlider* p = this;

  // area
  QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
  groove.adjust(0, 0, -1, 0);

  // pen & brush
  QLinearGradient gradient(groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
  gradient.setColorAt(0, color.dark(0));
  gradient.setColorAt(1, color.light(108));
  painter->setBrush(gradient);
  painter->setPen(QPen(color.dark(130), 0));

  // draw groove
  painter->drawRect(rect.intersected(groove));
}

void CompositionControlSlider::drawHandle(QStylePainter* painter, CompositionControlSlider::SpanHandle handle) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QStyleOptionSlider opt;
  initStyleOption(&opt, handle);
  opt.subControls = QStyle::SC_SliderHandle;
  QStyle::SubControl pressed;

  switch (handle) {
    case CompositionControlSlider::LRNC_lowerHandle:
      pressed = LRNC_lowerPressed;
      break;
    case CompositionControlSlider::LRNC_upperHandle:
      pressed = LRNC_upperPressed;
      break;
    case CompositionControlSlider::CALC_lowerHandle:
      pressed = CALC_lowerPressed;
      break;
    case CompositionControlSlider::CALC_upperHandle:
      pressed = CALC_upperPressed;
      break;
    case CompositionControlSlider::NoHandle:
      break;
  }

  if (pressed == QStyle::SC_SliderHandle) {
    opt.activeSubControls = pressed;
    opt.state |= QStyle::State_Sunken;
  }
  painter->drawComplexControl(QStyle::CC_Slider, opt);
}

void CompositionControlSlider::triggerAction(QAbstractSlider::SliderAction action)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  switch (action) {
    case QAbstractSlider::SliderMove:
    case QAbstractSlider::SliderNoAction:
      break;
    default:
      qWarning("CompositionControlSlider::triggerAction: Unknown action");
      break;
  }
  blockTracking = false;
}

void CompositionControlSlider::movePressedHandle()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  triggerAction(QAbstractSlider::SliderMove);
}

void CompositionControlSlider::setLRNC_lowerValue(int value)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(value) << std::endl;

  // first process the new value directly
  LRNCLowerLimitSpinBox->setValue(qMax(value, LRNCLowerLimitSpinBox->minimum()));
  LRNCUpperLimitSpinBox->setMinimum(LRNCLowerLimitSpinBox->value()+1);
  emit valueChanged();

  // and then update the slider display
  LRNC_lowerPos = 0; // default before looking
  for (int i=0; i < HUvalueLUT.size(); i++) {
    int HUvalue = HUvalueLUT.at(i);
    if (HUvalue >= LRNCLowerLimitSpinBox->value()) {
      setLRNC_lowerPos(HUvalueLUT.indexOf(HUvalue), false);
      break;
    }
  }
  update();
}

void CompositionControlSlider::setCALC_lowerValue(int value)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(value) << std::endl;
  
  // first process the new value directly
  CALCLowerLimitSpinBox->setValue(qBound(LRNCUpperLimitSpinBox->value(), value, CALCUpperLimitSpinBox->value()));
  CALCUpperLimitSpinBox->setMinimum(CALCLowerLimitSpinBox->value()+1);
  LRNCUpperLimitSpinBox->setMaximum(CALCLowerLimitSpinBox->value()-1);
  emit valueChanged();

  // and then update the slider display
  CALC_lowerPos = 0; // default before looking
  for (int i=0; i < HUvalueLUT.size(); i++) {
    int HUvalue = HUvalueLUT.at(i);
    if (HUvalue >= CALCLowerLimitSpinBox->value()) {
      setCALC_lowerPos(HUvalueLUT.indexOf(HUvalue), false);
      break;
    }
  }
  update();
}

void CompositionControlSlider::setLRNC_upperValue(int value)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(value) << std::endl;

  // first process the new value directly
  LRNCUpperLimitSpinBox->setValue(qBound(LRNCLowerLimitSpinBox->value(), value, CALCLowerLimitSpinBox->value()));
  CALCLowerLimitSpinBox->setMinimum(LRNCUpperLimitSpinBox->value()+1);
  LRNCLowerLimitSpinBox->setMaximum(LRNCUpperLimitSpinBox->value()-1);
  emit valueChanged();

  // and then update the slider display
  LRNC_upperPos = 0; // default before looking
  for (int i=0; i < HUvalueLUT.size(); i++) {
    int HUvalue = HUvalueLUT.at(i);
    if (HUvalue >= LRNCUpperLimitSpinBox->value()) {
      setLRNC_upperPos(HUvalueLUT.indexOf(HUvalue), false);
      break;
    }
  }
  update();
}

void CompositionControlSlider::setCALC_upperValue(int value)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(value) << std::endl;
  
  // first process the new value directly
  CALCUpperLimitSpinBox->setValue(qMin(value, CALCUpperLimitSpinBox->maximum()));
  CALCLowerLimitSpinBox->setMaximum(CALCUpperLimitSpinBox->value()-1);
  emit valueChanged();

  // and then update the slider display
  CALC_upperPos = 0; // default before looking
  for (int i=0; i < HUvalueLUT.size(); i++) {
    int HUvalue = HUvalueLUT.at(i);
    if (HUvalue >= CALCUpperLimitSpinBox->value()) {
      setCALC_upperPos(HUvalueLUT.indexOf(HUvalue), false);
      break;
    }
  }
  update();
}

void CompositionControlSlider::setLRNC_lowerPos(int pos, bool alsoChangeValue)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(pos) << std::endl;
  if (LRNC_lowerPos != pos) {
    LRNC_lowerPos = pos;
    if (!hasTracking())
      update();
    if (hasTracking() && !blockTracking)
      triggerAction(SliderMove);
    if (alsoChangeValue) {
      LRNCLowerLimitSpinBox->setValue(HUvalueLUT.at(LRNC_lowerPos));
      emit valueChanged();
    }
  }
}

void CompositionControlSlider::setCALC_lowerPos(int pos, bool alsoChangeValue)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(pos) << std::endl;
  if (CALC_lowerPos != pos) {
    CALC_lowerPos = pos;
    if (!hasTracking())
      update();
    if (hasTracking() && !blockTracking)
      triggerAction(SliderMove);
    if (alsoChangeValue) {
      CALCLowerLimitSpinBox->setValue(HUvalueLUT.at(CALC_lowerPos));
      emit valueChanged();
    }
  }
}

void CompositionControlSlider::setLRNC_upperPos(int pos, bool alsoChangeValue)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(pos) << std::endl;
  if (LRNC_upperPos != pos) {
    LRNC_upperPos = pos;
    if (!hasTracking())
      update();
    if (hasTracking() && !blockTracking)
      triggerAction(SliderMove);
    if (alsoChangeValue) {
      LRNCUpperLimitSpinBox->setValue(HUvalueLUT.at(LRNC_upperPos));
      emit valueChanged();
    }
  }
}

void CompositionControlSlider::setCALC_upperPos(int pos, bool alsoChangeValue)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(pos) << std::endl;
  if (CALC_upperPos != pos) {
    CALC_upperPos = pos;
    if (!hasTracking())
      update();
    if (hasTracking() && !blockTracking)
      triggerAction(SliderMove);
    if (alsoChangeValue) {
      CALCUpperLimitSpinBox->setValue(HUvalueLUT.at(CALC_upperPos));
      emit valueChanged();
    }
  }
}

/*!
  \reimp
 */
void CompositionControlSlider::mousePressEvent(QMouseEvent* event)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (minimum() == maximum() || (event->buttons() ^ event->button())) {
    event->ignore();
    return;
  }
  QStyleOptionSlider opt;
  if ((LRNC_lowerPressed != QStyle::SC_SliderHandle) 
      && (CALC_lowerPressed != QStyle::SC_SliderHandle) 
      && (CALC_upperPressed != QStyle::SC_SliderHandle)) {
    initStyleOption(&opt, CompositionControlSlider::LRNC_upperHandle);
    const QStyle::SubControl oldControl = LRNC_upperPressed;
    LRNC_upperPressed = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, event->pos(), this);
    const QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    if (LRNC_upperPressed == QStyle::SC_SliderHandle) {
      position = LRNCUpperLimitSpinBox->value();
      offset = pick(event->pos() - sr.topLeft());
      lastPressed = CompositionControlSlider::LRNC_upperHandle;
      setSliderDown(true);
    }
    if (LRNC_upperPressed != oldControl)
      update(sr);
  }

  if ((LRNC_upperPressed != QStyle::SC_SliderHandle) 
      && (CALC_lowerPressed != QStyle::SC_SliderHandle) 
      && (CALC_upperPressed != QStyle::SC_SliderHandle)) {
    initStyleOption(&opt, CompositionControlSlider::LRNC_lowerHandle);
    const QStyle::SubControl oldControl = LRNC_lowerPressed;
    LRNC_lowerPressed = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, event->pos(), this);
    const QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    if (LRNC_lowerPressed == QStyle::SC_SliderHandle) {
      position = LRNCLowerLimitSpinBox->value();
      offset = pick(event->pos() - sr.topLeft());
      lastPressed = CompositionControlSlider::LRNC_lowerHandle;
      setSliderDown(true);
    }
    if (LRNC_lowerPressed != oldControl)
      update(sr);
  }

  if ((LRNC_upperPressed != QStyle::SC_SliderHandle) 
      && (LRNC_lowerPressed != QStyle::SC_SliderHandle) 
      && (CALC_upperPressed != QStyle::SC_SliderHandle)) {
    initStyleOption(&opt, CompositionControlSlider::CALC_lowerHandle);
    const QStyle::SubControl oldControl = CALC_lowerPressed;
    CALC_lowerPressed = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, event->pos(), this);
    const QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    if (CALC_lowerPressed == QStyle::SC_SliderHandle) {
      position = CALCLowerLimitSpinBox->value();
      offset = pick(event->pos() - sr.topLeft());
      lastPressed = CompositionControlSlider::CALC_lowerHandle;
      setSliderDown(true);
    }
    if (CALC_lowerPressed != oldControl)
      update(sr);
  }

  if ((CALC_lowerPressed != QStyle::SC_SliderHandle) 
      && (LRNC_lowerPressed != QStyle::SC_SliderHandle) 
      && (LRNC_upperPressed != QStyle::SC_SliderHandle)) {
    initStyleOption(&opt, CompositionControlSlider::CALC_upperHandle);
    const QStyle::SubControl oldControl = CALC_upperPressed;
    CALC_upperPressed = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, event->pos(), this);
    const QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    if (CALC_upperPressed == QStyle::SC_SliderHandle) {
      position = CALCUpperLimitSpinBox->value();
      offset = pick(event->pos() - sr.topLeft());
      lastPressed = CompositionControlSlider::CALC_upperHandle;
      setSliderDown(true);
    }
    if (CALC_upperPressed != oldControl)
      update(sr);
  }

  event->accept();
}

/*!
  \reimp
 */
void CompositionControlSlider::mouseMoveEvent(QMouseEvent* event)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
  int newPosition = pixelPosToRangeValue(pick(event->pos()) - offset);
  if (m >= 0) {
    const QRect r = rect().adjusted(-m, -m, m, m);
    if (!r.contains(event->pos()))
      newPosition = position;
  }

  if (CALC_lowerPressed == QStyle::SC_SliderHandle) {
    newPosition = qMax(newPosition, LRNC_upperPos + 10);
    newPosition = qMin(newPosition, CALC_upperPos - 10);
    newPosition = qMax(newPosition, minimum());
    newPosition = qMin(newPosition, maximum());
    setCALC_lowerPos(newPosition, true);
  }
  else if (CALC_upperPressed == QStyle::SC_SliderHandle) {
    newPosition = qMax(newPosition, CALC_lowerPos + 10);
    newPosition = qMax(newPosition, minimum());
    newPosition = qMin(newPosition, maximum());
    setCALC_upperPos(newPosition, true);
  }
  else if (LRNC_lowerPressed == QStyle::SC_SliderHandle) {
    newPosition = qMin(newPosition, LRNC_upperPos - 10);
    newPosition = qMax(newPosition, minimum());
    newPosition = qMin(newPosition, maximum());
    setLRNC_lowerPos(newPosition, true);
  }
  else if (LRNC_upperPressed == QStyle::SC_SliderHandle) {
    newPosition = qMax(newPosition, LRNC_lowerPos + 10);
    newPosition = qMin(newPosition, CALC_lowerPos - 10);
    newPosition = qMax(newPosition, minimum());
    newPosition = qMin(newPosition, maximum());
    setLRNC_upperPos(newPosition, true);
  }

  event->accept();
  //update();
}

/*!
  \reimp
 */
void CompositionControlSlider::mouseReleaseEvent(QMouseEvent* event)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QSlider::mouseReleaseEvent(event);
  setSliderDown(false);
  LRNC_lowerPressed = QStyle::SC_None;
  LRNC_upperPressed = QStyle::SC_None;
  CALC_lowerPressed = QStyle::SC_None;
  CALC_upperPressed = QStyle::SC_None;
  update();
}

/*!
  \reimp
 */
void CompositionControlSlider::paintEvent(QPaintEvent* event)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  Q_UNUSED(event);
  QStylePainter painter(this);

  // groove & ticks
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  opt.sliderPosition = 0;
  opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
  painter.drawComplexControl(QStyle::CC_Slider, opt);

  // LRNC handle rects
  opt.sliderPosition = LRNC_lowerPos;
  QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  int lrv  = pick(lr.center());
  opt.sliderPosition = LRNC_upperPos;
  QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  int urv  = pick(ur.center());

  // LRNC span
  int minv = qMin(lrv, urv);
  int maxv = qMax(lrv, urv);
  QPoint c = QRect(lr.center(), ur.center()).center();
  QRect spanRect = QRect(QPoint(minv, c.y() - 12), QPoint(maxv, c.y() + 9));
  drawSpan(&painter, spanRect, QColor(0xFF,0xFF,0x00, 255));

  // CALC lower rect
  opt.sliderPosition = CALC_lowerPos;
  lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  lrv  = pick(lr.center());

  /*// MATX span (Note, this span is implicit rather than explicit, from ur as left by LRNC, and lr as just calculated for CALC)
  minv = qMin(lrv, urv);
  maxv = qMax(lrv, urv);
  c = QRect(lr.center(), ur.center()).center();
  spanRect = QRect(QPoint(minv, c.y() - 12), QPoint(maxv, c.y() + 9));
  drawSpan(&painter, spanRect, QColor(0x10,0x10,0xFF, 255));*/

  // CALC upper rect
  opt.sliderPosition = CALC_upperPos;
  ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  urv  = pick(ur.center());

  // CALC span
  minv = qMin(lrv, urv);
  maxv = qMax(lrv, urv);
  c = QRect(lr.center(), ur.center()).center();
  spanRect = QRect(QPoint(minv, c.y() - 12), QPoint(maxv, c.y() + 9));
  drawSpan(&painter, spanRect, QColor(0x33,0xCC,0x99, 255));

  // handles
  switch (lastPressed) {
    case CompositionControlSlider::LRNC_lowerHandle:
      drawHandle(&painter, CompositionControlSlider::LRNC_upperHandle);
      drawHandle(&painter, CompositionControlSlider::LRNC_lowerHandle);
      drawHandle(&painter, CompositionControlSlider::CALC_lowerHandle);
      drawHandle(&painter, CompositionControlSlider::CALC_upperHandle);
      break;
    case CompositionControlSlider::LRNC_upperHandle:
      drawHandle(&painter, CompositionControlSlider::LRNC_lowerHandle);
      drawHandle(&painter, CompositionControlSlider::LRNC_upperHandle);
      drawHandle(&painter, CompositionControlSlider::CALC_lowerHandle);
      drawHandle(&painter, CompositionControlSlider::CALC_upperHandle);
      break;
    case CompositionControlSlider::CALC_lowerHandle:
      drawHandle(&painter, CompositionControlSlider::CALC_upperHandle);
      drawHandle(&painter, CompositionControlSlider::CALC_lowerHandle);
      drawHandle(&painter, CompositionControlSlider::LRNC_upperHandle);
      drawHandle(&painter, CompositionControlSlider::LRNC_lowerHandle);
      break;
    case CompositionControlSlider::CALC_upperHandle:
    default:
      drawHandle(&painter, CompositionControlSlider::CALC_lowerHandle);
      drawHandle(&painter, CompositionControlSlider::CALC_upperHandle);
      drawHandle(&painter, CompositionControlSlider::LRNC_upperHandle);
      drawHandle(&painter, CompositionControlSlider::LRNC_lowerHandle);
      break;
  }
}

CompositionControl::CompositionControl(QWidget *p, QString bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = p;
  oneOrMoreValuesChanged = false;
  message = new QErrorMessage(this);
  message->setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  message->setModal(true);
  Dialog = new QDialog(owner);
  Dialog->resize(343, 254);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(Dialog->sizePolicy().hasHeightForWidth());
  Dialog->setSizePolicy(sizePolicy);
  verticalLayout = new QVBoxLayout(Dialog);
  verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
  compositionControlSlider = new CompositionControlSlider(Dialog);
  compositionControlSlider->setObjectName(QStringLiteral("compositionControlSlider"));
  QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(compositionControlSlider->sizePolicy().hasHeightForWidth());
  compositionControlSlider->setSizePolicy(sizePolicy1);
  compositionControlSlider->setMinimumSize(QSize(326, 50));
  compositionControlSlider->setMaximumSize(QSize(326, 50));
  compositionControlSlider->setOrientation(Qt::Horizontal);

  verticalLayout->addWidget(compositionControlSlider);

  formLayout = new QFormLayout();
  formLayout->setObjectName(QStringLiteral("formLayout"));
  compositionControlSlider->LRNCLowerLimitLabel = new QLabel(Dialog);
  compositionControlSlider->LRNCLowerLimitLabel->setObjectName(QStringLiteral("LRNCLowerLimitLabel"));

  formLayout->setWidget(0, QFormLayout::LabelRole, compositionControlSlider->LRNCLowerLimitLabel);

  compositionControlSlider->LRNCLowerLimitSpinBox = new QSpinBox(Dialog);
  compositionControlSlider->LRNCLowerLimitSpinBox->setObjectName(QStringLiteral("LRNCLowerLimitSpinBox"));

  formLayout->setWidget(0, QFormLayout::FieldRole, compositionControlSlider->LRNCLowerLimitSpinBox);

  compositionControlSlider->LRNCUpperLimitLabel = new QLabel(Dialog);
  compositionControlSlider->LRNCUpperLimitLabel->setObjectName(QStringLiteral("LRNCUpperLimitLabel"));

  formLayout->setWidget(1, QFormLayout::LabelRole, compositionControlSlider->LRNCUpperLimitLabel);

  compositionControlSlider->LRNCUpperLimitSpinBox = new QSpinBox(Dialog);
  compositionControlSlider->LRNCUpperLimitSpinBox->setObjectName(QStringLiteral("LRNCUpperLimitSpinBox"));

  formLayout->setWidget(1, QFormLayout::FieldRole, compositionControlSlider->LRNCUpperLimitSpinBox);

  compositionControlSlider->CALCLowerLimitLabel = new QLabel(Dialog);
  compositionControlSlider->CALCLowerLimitLabel->setObjectName(QStringLiteral("CALCLowerLimitLabel"));

  formLayout->setWidget(2, QFormLayout::LabelRole, compositionControlSlider->CALCLowerLimitLabel);

  compositionControlSlider->CALCLowerLimitSpinBox = new QSpinBox(Dialog);
  compositionControlSlider->CALCLowerLimitSpinBox->setObjectName(QStringLiteral("CALCLowerLimitSpinBox"));

  formLayout->setWidget(2, QFormLayout::FieldRole, compositionControlSlider->CALCLowerLimitSpinBox);

  compositionControlSlider->CALCUpperLimitLabel = new QLabel(Dialog);
  compositionControlSlider->CALCUpperLimitLabel->setObjectName(QStringLiteral("CALCUpperLimitLabel"));

  formLayout->setWidget(3, QFormLayout::LabelRole, compositionControlSlider->CALCUpperLimitLabel);

  compositionControlSlider->CALCUpperLimitSpinBox = new QSpinBox(Dialog);
  compositionControlSlider->CALCUpperLimitSpinBox->setObjectName(QStringLiteral("CALCUpperLimitSpinBox"));

  formLayout->setWidget(3, QFormLayout::FieldRole, compositionControlSlider->CALCUpperLimitSpinBox);

  verticalLayout->addLayout(formLayout);

  buttonBox = new QDialogButtonBox(Dialog);
  buttonBox->setObjectName(QStringLiteral("buttonBox"));
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  verticalLayout->addWidget(buttonBox);

  QMetaObject::connectSlotsByName(Dialog);

  compositionControlSlider->LRNCLowerLimitLabel->setText(QApplication::translate("Dialog", "LRNC Lower Limit", 0));
  compositionControlSlider->LRNCUpperLimitLabel->setText(QApplication::translate("Dialog", "LRNC Upper Limit", 0));
  compositionControlSlider->CALCLowerLimitLabel->setText(QApplication::translate("Dialog", "CALC Lower Limit", 0));
  compositionControlSlider->CALCUpperLimitLabel->setText(QApplication::translate("Dialog", "CALC Upper Limit", 0));

  compositionControlSlider->LRNCLowerLimitSpinBox->setMinimum(-1018);
  compositionControlSlider->LRNCLowerLimitSpinBox->setMaximum(29);
  compositionControlSlider->LRNCLowerLimitSpinBox->installEventFilter(this); // used to trap use of enter key, which do not want to inadvertently result in triggerring re-processing
  compositionControlSlider->LRNCLowerLimitSpinBox->setValue(-200); // will be updated later, but need an initial plausible value for bounding when real value does come in
  
  compositionControlSlider->LRNCUpperLimitSpinBox->setMinimum(-201);
  compositionControlSlider->LRNCUpperLimitSpinBox->setMaximum(199);
  compositionControlSlider->LRNCUpperLimitSpinBox->installEventFilter(this); // used to trap use of enter key, which do not want to inadvertently result in triggerring re-processing
  compositionControlSlider->LRNCUpperLimitSpinBox->setValue(30); // will be updated later, but need an initial plausible value for bounding when real value does come in
  
  compositionControlSlider->CALCLowerLimitSpinBox->setMinimum(31);
  compositionControlSlider->CALCLowerLimitSpinBox->setMaximum(1999);
  compositionControlSlider->CALCLowerLimitSpinBox->installEventFilter(this); // used to trap use of enter key, which do not want to inadvertently result in triggerring re-processing
  compositionControlSlider->CALCLowerLimitSpinBox->setValue(200); // will be updated later, but need an initial plausible value for bounding when real value does come in
  
  compositionControlSlider->CALCUpperLimitSpinBox->setMinimum(201);
  compositionControlSlider->CALCUpperLimitSpinBox->setMaximum(5068);
  compositionControlSlider->CALCUpperLimitSpinBox->installEventFilter(this); // used to trap use of enter key, which do not want to inadvertently result in triggerring re-processing
  compositionControlSlider->CALCUpperLimitSpinBox->setValue(2000); // will be updated later, but need an initial plausible value for bounding when real value does come in
  
  Dialog->setObjectName(QStringLiteral("CompositionControl"));
  Dialog->setWindowTitle(tr("Tissue Component HU Limits for ")+bodySite);
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Re-Process Using These Settings"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Dismiss"));
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptCompositionSettings()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelCompositionSetting()));
  Dialog->setWindowFlags(Dialog->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
  QObject::connect(compositionControlSlider, SIGNAL(valueChanged()), this, SLOT(indicateThatChangeHasBeenMadeButNotYetProcessed()));
  QMetaObject::connectSlotsByName(Dialog);
  Dialog->setFocus();

  // preliminary connects
  connect(compositionControlSlider->LRNCLowerLimitSpinBox, SIGNAL(valueChanged(int)), compositionControlSlider, SLOT(setLRNC_lowerValue(int)));
  connect(compositionControlSlider->LRNCUpperLimitSpinBox, SIGNAL(valueChanged(int)), compositionControlSlider, SLOT(setLRNC_upperValue(int)));
  connect(compositionControlSlider->CALCLowerLimitSpinBox, SIGNAL(valueChanged(int)), compositionControlSlider, SLOT(setCALC_lowerValue(int)));
  connect(compositionControlSlider->CALCUpperLimitSpinBox, SIGNAL(valueChanged(int)), compositionControlSlider, SLOT(setCALC_upperValue(int)));
  connect(compositionControlSlider, SIGNAL(sliderReleased()), compositionControlSlider, SLOT(movePressedHandle()));
}

void CompositionControl::indicateThatChangeHasBeenMadeButNotYetProcessed()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  oneOrMoreValuesChanged = true;
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  Dialog->repaint();
  qApp->processEvents();
}

void CompositionControl::acceptCompositionSettings()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  Dialog->hide();
  // need to set the parameter values and re-do the computations implied by which ones changed.
  if (oneOrMoreValuesChanged) {
    // first update the settings per the spin boxes...
    int index = ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("LRNC_lower_limit");
    if (index == -1)
      message->showMessage(tr("missing limit parameter, contact Elucid for assistance"));
    else
      ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settingEntries.at(index)->setText(QString::number(compositionControlSlider->LRNCLowerLimitSpinBox->value()));
    index = ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("LRNC_upper_limit");
    if (index == -1)
      message->showMessage(tr("missing limit parameter, contact Elucid for assistance"));
    else 
      ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settingEntries.at(index)->setText(QString::number(compositionControlSlider->LRNCUpperLimitSpinBox->value()));
    index = ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("CALC_lower_limit");
    if (index == -1)
      message->showMessage(tr("missing limit parameter, contact Elucid for assistance"));
    else
      ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settingEntries.at(index)->setText(QString::number(compositionControlSlider->CALCLowerLimitSpinBox->value()));
    index = ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("CALC_upper_limit");
    if (index == -1)
      message->showMessage(tr("missing limit parameter, contact Elucid for assistance"));
    else
      ((processingParameters *)owner)->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settingEntries.at(index)->setText(QString::number(compositionControlSlider->CALCUpperLimitSpinBox->value()));

    // ...and now cause the re-computation
    ((processingParameters *)owner)->acceptParameterSettings(); // first clear out the pipeline including and after composition
    ((processingParameters *)owner)->emitProcessCompositionSettingsChange();
    // can't do anything after this because the control will be deleted by the emit above
  }
}

void CompositionControl::cancelCompositionSetting()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // since we don't push new values until confirmation, nothing to do here but hide
  oneOrMoreValuesChanged = false;
  if (Dialog)
    if (Dialog->isVisible())
      Dialog->hide();
  processingParameters *processingParametersOwner = (processingParameters *)owner;
  processingParametersOwner->compositionControlCurrentlyInUse = false;
}

bool CompositionControl::eventFilter(QObject *obj, QEvent *event) 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // one type of event we need to capture are those coming from the spin boxes
  if (obj == compositionControlSlider->LRNCLowerLimitSpinBox) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *key = static_cast<QKeyEvent*>(event);
      if ((key->key() == Qt::Key_Enter) || ((key->key() == Qt::Key_Return)))
        return true; // this ends up causing it to be stopped here
    }
  }
  else if (obj == compositionControlSlider->LRNCUpperLimitSpinBox) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *key = static_cast<QKeyEvent*>(event);
      if ((key->key() == Qt::Key_Enter) || ((key->key() == Qt::Key_Return)))
        return true; // this ends up causing it to be stopped here
    }
  }
  else if (obj == compositionControlSlider->CALCLowerLimitSpinBox) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *key = static_cast<QKeyEvent*>(event);
      if ((key->key() == Qt::Key_Enter) || ((key->key() == Qt::Key_Return)))
        return true; // this ends up causing it to be stopped here
    }
  }
  else if (obj == compositionControlSlider->CALCUpperLimitSpinBox) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *key = static_cast<QKeyEvent*>(event);
      if ((key->key() == Qt::Key_Enter) || ((key->key() == Qt::Key_Return)))
        return true; // this ends up causing it to be stopped here
    }
  }

  // all events reaching this stage are passed further on
  return false; 
}

processingParameters::processingParameters(QWidget *p, systemPreferences *prefObj, QString juris, ebiVesselPipeline::Pointer pline, QString b, ebID tgtPid)
{
  ebLog eblog(Q_FUNC_INFO); eblog << b.toStdString() << std::endl;
  owner = p;
  clinicalJurisdiction = juris;
  systemPreferencesObject = prefObj;
  alreadyReceivedConfirmation = false;
  compositionControlCurrentlyInUse = false;
  pipeline = pline;
  bodySite = b;
  targetPipelineID = tgtPid; 
  compositionControl = new CompositionControl(this, b);
  message = new QErrorMessage(this);
  message->setWindowFlags(message->windowFlags()|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  message->setModal(true);
  Dialog = new QDialog(owner);
  Dialog->setObjectName(QStringLiteral("processingParameters"));
  Dialog->setWindowFlags(Dialog->windowFlags()|Qt::WindowStaysOnTopHint);
  Dialog->setModal(true);
  QString title = tr("Settings for processing subsequent computations are listed below and may be changed. (Provenance records capture settings used in any given computation.)");
  Dialog->setWindowTitle(title);
  Dialog->resize(326, 128);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(Dialog->sizePolicy().hasHeightForWidth());
  Dialog->setSizePolicy(sizePolicy);
  gridLayout_3 = new QGridLayout(Dialog);
  gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
  int k = 0;

  // now get the default values for each pipeline stage 
  while (1) {
    stageParameters *newStageParameters = new stageParameters();
    if (k==ebiVesselTargetPipeline::INITIALIZATION_STAGE) {
      newStageParameters->stageName = tr("TargetInitialization");
      newStageParameters->parameterString = "{}";//pipeline->GetVesselTargetPipeline(targetPipelineID)->GetInitializationParameters();
      newStageParameters->versionString = "FilterName 1.2.3 [vtk 7.0.0 itk 4.11.0]";//pipeline->GetVesselTargetPipeline(targetPipelineID)->GetInitializationFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE) {
      newStageParameters->stageName = tr("Lumen Segmentation");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenSegmentationParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenSegmentationFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::PATH_STAGE) {
      newStageParameters->stageName = tr("Target Path Finding");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPathParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPathFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LUMENPARTITION_STAGE) {
      newStageParameters->stageName = tr("Lumen Partitioning");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenPartitionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenPartitionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE) {
      newStageParameters->stageName = tr("Image Registration");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetRegistrationParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetRegistrationFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE) {
      newStageParameters->stageName = tr("Wall Segmentation");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallSegmentationParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallSegmentationFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE) {
      newStageParameters->stageName = tr("Wall Partitioning");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallPartitionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallPartitionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::WALLTHICKNESS_STAGE) {
      newStageParameters->stageName = tr("Wall Thickness");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetWallThicknessParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetWallThicknessFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE) {
      newStageParameters->stageName = tr("Perivascular Region");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE) {
      newStageParameters->stageName = tr("Perivascular Region Partitioning");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionPartitionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionPartitionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::COMPOSITION_STAGE) {
      newStageParameters->stageName = tr("Tissue Characteristics");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCompositionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCompositionFilter()->GetVersion();
      std::cerr << "default composition parameters" << newStageParameters->parameterString << std::endl;
      if (clinicalJurisdiction == "") { // only in research edition (which is signified by blank jurisdiction)
        // no overrides in this release. See git chnage on 8-2-2019 for example code of how to do so if needed in future.
        ;
      }
      std::cerr << "composition parameters after RE check" << newStageParameters->parameterString << std::endl;
    }
    else if (k==ebiVesselTargetPipeline::CAPTHICKNESS_STAGE) {
      newStageParameters->stageName = tr("Cap Thickness");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCapThicknessParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCapThicknessFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::READINGS_STAGE) {
      newStageParameters->stageName = tr("Quantitative Calculations");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetReadingsParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetReadingsFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE) {
      newStageParameters->stageName = tr("Lesion Lumen Partition");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenPartitionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenPartitionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE) {
      newStageParameters->stageName = tr("Lesion LumenAndWall Partition");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenAndWallPartitionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenAndWallPartitionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE) {
      newStageParameters->stageName = tr("Lesion Perivascular Region Partition");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionPerivascularRegionPartitionParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionPerivascularRegionPartitionFilter()->GetVersion();
    }
    else if (k==ebiVesselTargetPipeline::LESIONREADINGS_STAGE) {
      newStageParameters->stageName = tr("Lesion Quantitative Calculations");
      newStageParameters->parameterString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionReadingsParameters();
      newStageParameters->versionString = pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionReadingsFilter()->GetVersion();
    }
    else 
      break;

    // now that we have all the values, format the dialog box
    newStageParameters->stageBox = new QGroupBox(Dialog);
    QString stageBoxLegend = newStageParameters->stageName;
    stageBoxLegend.append(", ");
    newStageParameters->versionSubString = QString::fromStdString(newStageParameters->versionString).split(' ');
    newStageParameters->version = newStageParameters->versionSubString.at(1);
    stageBoxLegend.append(newStageParameters->version);
    newStageParameters->stageBox->setTitle(stageBoxLegend);
    newStageParameters->stageBox->setObjectName(QStringLiteral("groupBox"));
    sizePolicy.setHeightForWidth(newStageParameters->stageBox->sizePolicy().hasHeightForWidth());
    newStageParameters->stageBox->setSizePolicy(sizePolicy);
    newStageParameters->parameters = new QVBoxLayout(newStageParameters->stageBox);
    newStageParameters->parameters->setObjectName(QStringLiteral("gridLayout_2"));
    newStageParameters->byteArray = newStageParameters->parameterString.c_str();
    newStageParameters->doc = QJsonDocument::fromJson(newStageParameters->byteArray, &newStageParameters->err);
    if (newStageParameters->doc.isObject()) {
      newStageParameters->list = newStageParameters->doc.object();
      int i=0;
      foreach (const QString key, newStageParameters->list.keys()) {
        if ((clinicalJurisdiction != "") && key.contains("IPH")) // only in research edition (which is signified by blank jurisdiction)
          continue;
        newStageParameters->parameterKeys << key;
        newStageParameters->settings << newStageParameters->list[key].toDouble(); 
        newStageParameters->parameterPrompts << new QLabel(newStageParameters->stageBox);
        ((newStageParameters->parameterPrompts)[i])->setObjectName(QStringLiteral("parameterPrompt"));
        if (i == 0)
          newStageParameters->formLayout_5 = new QFormLayout(newStageParameters->stageBox);
        newStageParameters->formLayout_5->setWidget(i, QFormLayout::LabelRole, (newStageParameters->parameterPrompts)[i]);
        newStageParameters->settingEntries << new QLineEdit(newStageParameters->stageBox);
        ((newStageParameters->settingEntries)[i])->setValidator(new QDoubleValidator(this));
        ((newStageParameters->settingEntries)[i])->setObjectName(key);
        ((newStageParameters->settingEntries)[i])->setAlignment(Qt::AlignRight);
        ((newStageParameters->settingEntries)[i])->setMinimumWidth(50); 
        newStageParameters->formLayout_5->setWidget(i, QFormLayout::FieldRole, (newStageParameters->settingEntries)[i]);
        ((newStageParameters->settingEntries)[i])->setText(QString::number((newStageParameters->settings)[i]));
        ((newStageParameters->parameterPrompts)[i])->setText(key);
        ((newStageParameters->settingEntries)[i])->installEventFilter(this); // adds linkage to event filter for confirming user intention
        if (i == 0)
          newStageParameters->parameters->addLayout(newStageParameters->formLayout_5, i);
        i++;
      }
      if (newStageParameters->parameterKeys.size() > 0)
        gridLayout_3->addWidget(newStageParameters->stageBox);
      else
        delete newStageParameters->stageBox;
    }
    if (newStageParameters->parameterKeys.size() > 0) {
      newStageParameters->parameters->addStretch(1);
      gridLayout_3->addWidget(newStageParameters->stageBox, 0, k, 1, 1);
    }
    parametersByStages.append(newStageParameters);
    k++;
  } // end while there are more stages

  buttonBox = new QDialogButtonBox(Dialog);
  buttonBox->setObjectName(QStringLiteral("buttonBox"));
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  gridLayout_3->addWidget(buttonBox, 1, 0, 1, k); // was 1, 0, 1, 2
  
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptParameterSettings()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelParameterSetting()));
  QMetaObject::connectSlotsByName(Dialog);
  Dialog->setFocus();
}

void processingParameters::presentDialog()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  Dialog->show();
}

void processingParameters::acceptParameterSettings()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  // need to set the parameter values and re-do the computations implied by which ones changed.
  // loop over the stages from earliest to latest, comparing the objects until find one that changed and then emit the signal accordingly.
  int earliestStage = 9999;
  for (int k=parametersByStages.size()-1; k >= 0; k--) {
    bool atLeastOneSettingChangedForStage = false;
    for (int i=0; i < parametersByStages.at(k)->settings.size(); i++) {
      if (parametersByStages.at(k)->settings.at(i) != ((parametersByStages.at(k)->settingEntries)[i])->text().toDouble()) {
        earliestStage = k;
        parametersByStages.at(k)->settings[i] = ((parametersByStages.at(k)->settingEntries)[i])->text().toDouble();
        atLeastOneSettingChangedForStage = true;
      }
    }
    if (atLeastOneSettingChangedForStage) {
      QJsonObject settingsObject;
      for (int i=0; i < parametersByStages.at(k)->settings.size(); i++) {
        settingsObject[parametersByStages.at(k)->parameterKeys.at(i)] = parametersByStages.at(k)->settings.at(i);
      }
      QJsonDocument doc(settingsObject);
      QString parameterQString(doc.toJson(QJsonDocument::Compact)); 
      switch (k) {
        case ebiVesselTargetPipeline::INITIALIZATION_STAGE: 
          //pipeline->GetVesselTargetPipeline(targetPipelineID)->SetInitializationParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLumenSegmentationParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::PATH_STAGE: 
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetPathParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LUMENPARTITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLumenPartitionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetRegistrationParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLumenAndWallSegmentationParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLumenAndWallPartitionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::WALLTHICKNESS_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetWallThicknessParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetPerivascularRegionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetPerivascularRegionPartitionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::COMPOSITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetCompositionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::CAPTHICKNESS_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetCapThicknessParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::READINGS_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetReadingsParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLesionLumenPartitionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLesionLumenAndWallPartitionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLesionPerivascularRegionPartitionParameters(parameterQString.toStdString());
          break;
        case ebiVesselTargetPipeline::LESIONREADINGS_STAGE:
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SetLesionReadingsParameters(parameterQString.toStdString());
          break;
      }
    }
  }
  qDebug() << "ending earliestStage is now " << earliestStage;

  /*if (earliestStage==ebiVesselTargetPipeline::INITIALIZATION_STAGE) {
      if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetInitialization())
        pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseInitialization(); 
  }
  else*/ if (earliestStage==ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenSegmentation()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLumenSegmentation(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::PATH_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPath()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->ClosePath(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LUMENPARTITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenPartition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLumenPartition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetResampledRegisteredImages()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseRegistration(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallSegmentation()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLumenAndWallSegmentation(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallPartition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLumenAndWallPartition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::WALLTHICKNESS_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetWallThickness()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseWallThickness(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegion()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->ClosePerivascularRegion(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionPartition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->ClosePerivascularRegionPartition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::COMPOSITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetComposition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseComposition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::CAPTHICKNESS_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCapThickness()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseCapThickness(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::READINGS_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetReadings()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseReadings(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenPartition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLesionLumenPartition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenAndWallPartition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLesionLumenAndWallPartition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionPerivascularRegionPartition()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLesionPerivascularRegionPartition(); 
    }
  }
  else if (earliestStage==ebiVesselTargetPipeline::LESIONREADINGS_STAGE) {
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionReadings()) {
      pipeline->GetVesselTargetPipeline(targetPipelineID)->CloseLesionReadings(); 
    }
  }
  if (earliestStage < 9999) {
    emit processingParametersSettingsChanged();  
  }
  Dialog->hide();
}

bool processingParameters::eventFilter(QObject *obj, QEvent *event) 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // check if it was one of the property values
  for (int k=0; k < parametersByStages.size(); k++) {
    for (int i=0; i < parametersByStages.at(k)->settingEntries.size(); i++) {
      if ((obj == parametersByStages.at(k)->settingEntries.at(i)) && (event->type() == QEvent::MouseButtonPress)) {
        if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
          return confirmParameterSettingChange();
        }
      }
    }
  }

  // reach here because it wasn't one of the stage boxes, so indicate it wasn't an event we are looking for in this filter
  return false;
}

bool processingParameters::confirmParameterSettingChange() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (systemPreferencesObject->getConfirmationWhenSetProcessingParameters()  && !alreadyReceivedConfirmation) {
    QMessageBox msgBox(this);
    msgBox.setWindowFlags(msgBox.windowFlags()|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    msgBox.setModal(true);
    msgBox.setText(tr("Changing processing parameter settings is a significant change, resulting in computations on the target being reset."));
    msgBox.setInformativeText(tr("Do you want to change settings anyway, resetting analyses?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No) {
      // return without action except for taking the dialog down
      Dialog->hide(); // hide the dialog, effectively cancelling the change
      return true; // true in this context means that the filter will stop it, not pass it on
    }
    else {
      alreadyReceivedConfirmation = true;
      return false; // false in this context means that the filter will pass it on to the original widget, i.e., the user really wants to change it, having confirmed it
    }
  }
  else
    return false; // false in this context means that the filter will pass it on to the original widget, i.e., the user really wants to change it, without prompting
}

void processingParameters::cancelParameterSetting()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // just take the dialog down (the user may have just wanted to review the settings, or otherwise decided not to change them)
  if (Dialog) {
    Dialog->hide();
  }
}

void processingParameters::presentCompositionControl()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  cancelParameterSetting(); // can't have this open and also the composition control
  compositionControlCurrentlyInUse = true;
  // first determine if we should actually do it; in order to do so, IPH computation has to be off and the preference for the dialog has to be true
  bool compute_IPH = false;
  int index = parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("compute_IPH");
  if (index != -1)
    compute_IPH = parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settings.at(index) == 0 ? false : true;
    
  if (systemPreferencesObject->getPresentCompositionControl() && compute_IPH) {
    // they want the control but are also computing IPH, so explictly advise them on how to make adjustments since control doesn't do IPH (the control could be enhanced later to do
    // it, but not important to do so now) (and no need for message in RE)
    if (clinicalJurisdiction != "") {
      message->showMessage(tr("Composition Control does not currently support IPH, please use processing parameters dialog directly to adjust limits"));
    }
  }
  else if (systemPreferencesObject->getPresentCompositionControl()) {
    // set up the defaults
    qApp->processEvents();
    disconnect(compositionControl->compositionControlSlider->LRNCLowerLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setLRNC_lowerValue(int)));
    disconnect(compositionControl->compositionControlSlider->LRNCUpperLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setLRNC_upperValue(int)));
    disconnect(compositionControl->compositionControlSlider->CALCLowerLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setCALC_lowerValue(int)));
    disconnect(compositionControl->compositionControlSlider->CALCUpperLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setCALC_upperValue(int)));
    disconnect(compositionControl->compositionControlSlider, SIGNAL(sliderReleased()), compositionControl->compositionControlSlider, SLOT(movePressedHandle()));
    index = parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("LRNC_lower_limit");
    if (index == -1)
      message->showMessage(tr("missing limit default, contact Elucid for assistance"));
    else
      compositionControl->compositionControlSlider->setLRNC_lowerValue(parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settings.at(index));
    index = parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("LRNC_upper_limit");
    if (index == -1)
      message->showMessage(tr("missing limit default, contact Elucid for assistance"));
    else
      compositionControl->compositionControlSlider->setLRNC_upperValue(parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settings.at(index));
    index = parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("CALC_lower_limit");
    if (index == -1)
      message->showMessage(tr("missing limit default, contact Elucid for assistance"));
    else
      compositionControl->compositionControlSlider->setCALC_lowerValue(parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settings.at(index));
    index = parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->parameterKeys.indexOf("CALC_upper_limit");
    if (index == -1)
      message->showMessage(tr("missing limit default, contact Elucid for assistance"));
    else
      compositionControl->compositionControlSlider->setCALC_upperValue(parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE)->settings.at(index));

    // now that the initializations are done, connect the signals for later updates
    qApp->processEvents();
    connect(compositionControl->compositionControlSlider->LRNCLowerLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setLRNC_lowerValue(int)));
    connect(compositionControl->compositionControlSlider->LRNCUpperLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setLRNC_upperValue(int)));
    connect(compositionControl->compositionControlSlider->CALCLowerLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setCALC_lowerValue(int)));
    connect(compositionControl->compositionControlSlider->CALCUpperLimitSpinBox, SIGNAL(valueChanged(int)), compositionControl->compositionControlSlider, SLOT(setCALC_upperValue(int)));
    connect(compositionControl->compositionControlSlider, SIGNAL(sliderReleased()), compositionControl->compositionControlSlider, SLOT(movePressedHandle()));

    // last but not least, present the dialog
    compositionControl->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false); // the initialization above will have interpreted the values as changes, but they aren't actually changes
    compositionControl->Dialog->show();
  }
}

void processingParameters::emitProcessCompositionSettingsChange()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  emit ((targetDefine *)owner)->processCompositionSettingsChange();
}
