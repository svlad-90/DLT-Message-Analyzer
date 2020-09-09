#ifndef CQTHELPER_HPP
#define CQTHELPER_HPP

#include <functional>

#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

template<typename T>
using tRangedArithmeticValueConverter = std::function<T(bool*, const QString&)>;

template<typename T>
bool getRangedArithmeticValue(T& result,
                    const T& minOverallVal,
                    const T& maxOverallVal,
                    const T& currentVal,
                    QWidget* pParent,
                    const QString& parameterNameNonCapital,
                    const QString& parameterNameCapital,
                    const tRangedArithmeticValueConverter<T>& converter)
{
    static_assert ( std::is_arithmetic<T>::value, "error: T should be an arithmetic type!" );

    bool bResult = false;

    if(converter)
    {
        QDialog dialog(pParent);
        QFormLayout form(&dialog);
        form.addRow(new QLabel(QString("Please, specify %1:").arg(parameterNameNonCapital)));

        QString labelFrom = QString("%1 ( from %2 to %3 ):").arg(parameterNameCapital).arg(minOverallVal).arg(maxOverallVal);
        QLineEdit *pCacheSizeLineEdit = new QLineEdit(&dialog);
        pCacheSizeLineEdit->setText(QString::number(currentVal));
        form.addRow(labelFrom, pCacheSizeLineEdit);

        // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, &dialog);

        auto acceptHandler = [&dialog,
                &pCacheSizeLineEdit,
                &minOverallVal,
                &maxOverallVal,
                &currentVal,
                &converter]()
        {
            bool bIsCacheSizeADigit = false;

            auto cacheSize = static_cast<T>(converter(&bIsCacheSizeADigit, pCacheSizeLineEdit->text()));

            bool bIsCacheSizeInMinRange = static_cast<T>(cacheSize) >= static_cast<T>(minOverallVal);
            bool bIsCacheSizeInMaxRange = static_cast<T>(cacheSize) <= static_cast<T>(maxOverallVal);

            bool areAllValuesValid = bIsCacheSizeADigit &&
                                     bIsCacheSizeInMinRange &&
                                     bIsCacheSizeInMaxRange;

            if(true == areAllValuesValid)
            {
                dialog.accept();
            }
            else
            {
                if(false == bIsCacheSizeADigit)
                {
                    pCacheSizeLineEdit->setText( QString::number(currentVal) );
                }
                else if(false == bIsCacheSizeInMinRange)
                {
                    pCacheSizeLineEdit->setText( QString::number(minOverallVal) );
                }
                else if(false == bIsCacheSizeInMaxRange)
                {
                    pCacheSizeLineEdit->setText( QString::number(maxOverallVal) );
                }
            }
        };

        auto rejectHandler = [&dialog]()
        {
            dialog.reject();
        };

        form.addRow(&buttonBox);
        QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, acceptHandler);
        QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, rejectHandler);

        // Show the dialog as modal
        if (dialog.exec() == QDialog::Accepted)
        {
            bool isCacheSizeADigit = false;
            auto tmpResult = static_cast<T>(converter(&isCacheSizeADigit, pCacheSizeLineEdit->text()));

            if(true == isCacheSizeADigit)
            {
                result = tmpResult;
                bResult = true;
            }
        }
    }

    return bResult;
}

#endif // CQTHELPER_HPP
