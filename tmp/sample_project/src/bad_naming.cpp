#include "bad_naming.hpp"

WrongName::WrongName() : wrongValue(0), badData(0) {}

void WrongName::SetValue(int val) {
    wrongValue = val;
    UpdateState();
}

int WrongName::GetValue() const {
    return calculateInternal();
}

int WrongName::calculateInternal() const {
    return wrongValue + badData;
}

void WrongName::UpdateState() {
    badData = wrongValue;
}
