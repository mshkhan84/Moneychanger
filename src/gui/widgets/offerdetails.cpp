#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/offerdetails.hpp>
#include <ui_offerdetails.h>

#include <gui/widgets/wizardnewoffer.hpp>
#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/modeltradearchive.hpp>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>
#include <QDateTime>
#include <QDebug>


// ------------------------------------------------------------------------

void MTOfferDetails::AddButtonClicked()
{
    Q_ASSERT(NULL != m_pOwner);
    // ---------------------------------------------------
    QString qstrMarketNymID    = m_pOwner->GetMarketNymID();
    QString qstrMarketNotaryID = m_pOwner->GetMarketNotaryID();
    // ---------------------------------------------------
    if (qstrMarketNymID.isEmpty())
    {
        QMessageBox::warning(this, tr("Need Nym ID"),
                             tr("Missing Nym ID. Please select it from the drop-down."));
        return;
    }
    // ---------------------------------------------------
    QString qstrMarketNymName = QString::fromStdString(opentxs::OT::App().API().Exec().GetNym_Name(qstrMarketNymID.toStdString()));
    QString qstrMarketServerName;
    // ---------------------------------------------------
    const QString qstrAll(tr("all"));

    if (qstrMarketNotaryID.isEmpty() || (0 == qstrAll.compare(qstrMarketNotaryID)))
    {
        if (false == ChooseServer(qstrMarketNotaryID, qstrMarketServerName))
            return;
    }
    else
        qstrMarketServerName = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(qstrMarketNotaryID.toStdString()));
    // ---------------------------------------------------
    WizardNewOffer theWizard(this);

    theWizard.SetNymID     (qstrMarketNymID);
    theWizard.SetNymName   (qstrMarketNymName);
    theWizard.SetNotaryID  (qstrMarketNotaryID);
    theWizard.SetServerName(qstrMarketServerName);
    // ---------------------------------------------------
    const std::string str_nym_id   (qstrMarketNymID   .toStdString());
    const std::string str_notary_id(qstrMarketNotaryID.toStdString());
    // ---------------------------------------------------
    theWizard.setWindowTitle(tr("Create Offer"));

    if (QDialog::Accepted == theWizard.exec())
    {
        // --------------------------------------------
        const bool    bIsBid                 (theWizard.field("bid")                .toBool());
        const bool    bSelling               (!bIsBid);
        // --------------------------------------------
        const bool    bIsMarketOrder         (theWizard.field("isMarketOrder")      .toBool());
        const bool    bIsFillOrKill          (theWizard.field("isFillOrKill")       .toBool());
        // --------------------------------------------
        const QString qstrInstrumentDefinitionID            (theWizard.field("InstrumentDefinitionID")            .toString());
        const QString qstrCurrencyID         (theWizard.field("CurrencyID")         .toString());
        // --------------------------------------------
        const std::string str_asset_id       (qstrInstrumentDefinitionID                           .toStdString());
        const std::string str_currency_id    (qstrCurrencyID                        .toStdString());
        // --------------------------------------------
//      const QString qstrAssetName          (theWizard.field("AssetName")          .toString());
//      const QString qstrCurrencyName       (theWizard.field("CurrencyName")       .toString());
        // --------------------------------------------
        const QString qstrQuantity           (theWizard.field("quantityAsset")      .toString());
        const int64_t lQuantity              (static_cast<int64_t>(qstrQuantity     .toLong  ())); // If this is "9"...
        // --------------------------------------------
        const QString qstrTotalAsset         (theWizard.field("totalAsset")         .toString());//...then this is "BTC 9.000"
        const int64_t lTotalAsset            (opentxs::OT::App().API().Exec().StringToAmount(str_asset_id,           //...and lTotalAsset is 9000
                                                                         qstrTotalAsset.toStdString())); // Note: scale is already inside this.
        // --------------------------------------------
        const QString qstrScale              (theWizard.field("scale")              .toString      ());// If this is "BTC 1.000"
        const int64_t lScale                 (opentxs::OT::App().API().Exec().StringToAmount(str_asset_id,                 // ...then lScale is 1000
                                                                         qstrScale  .toStdString   ()));
        // --------------------------------------------
        const QString qstrPrice              (theWizard.field("pricePerScale")      .toString      ());// If this is "$ 45.98"
        const int64_t lPrice                 (opentxs::OT::App().API().Exec().StringToAmount(str_currency_id,              // ...then lPrice is 4598
                                                                         qstrPrice  .toStdString   ())); // (per scale.)
        // --------------------------------------------
        const QString qstrAssetAcctID        (theWizard.field("AssetAcctID")        .toString());
        const QString qstrCurrencyAcctID     (theWizard.field("CurrencyAcctID")     .toString());
        // --------------------------------------------
        const std::string str_asset_acct_id   (qstrAssetAcctID                      .toStdString());
        const std::string str_currency_acct_id(qstrCurrencyAcctID                   .toStdString());
        // --------------------------------------------
        const QString qstrAssetAcctName      (theWizard.field("AssetAcctName")      .toString());
        const QString qstrCurrencyAcctName   (theWizard.field("CurrencyAcctName")   .toString());
        // --------------------------------------------
        const QString qstrAssetAcctBalance   (theWizard.field("AssetAcctBalance")   .toString());
        const QString qstrCurrencyAcctBalance(theWizard.field("CurrencyAcctBalance").toString());
        // --------------------------------------------
        const int64_t lAssetAcctBalance      (opentxs::OT::App().API().Exec().StringToAmount(str_asset_id,
                                                                         qstrAssetAcctBalance   .toStdString()));
        const int64_t lCurrencyAcctBalance   (opentxs::OT::App().API().Exec().StringToAmount(str_currency_id,
                                                                         qstrCurrencyAcctBalance.toStdString()));
        // --------------------------------------------
//      const QString qstrExpire             (theWizard.field("expirationStr")      .toString());
        const int     nExpire                (theWizard.field("expiration")         .toInt   ());
        // --------------------------------------------
        const int     timespan_array_len                  = 7;
        const int64_t array_timespan[timespan_array_len]  = {
            OT_TIME_MINUTE_IN_SECONDS,
            OT_TIME_HOUR_IN_SECONDS,
            OT_TIME_DAY_IN_SECONDS,
            OT_TIME_MONTH_IN_SECONDS,
            OT_TIME_THREE_MONTHS_IN_SECONDS,
            OT_TIME_SIX_MONTHS_IN_SECONDS,
            OT_TIME_YEAR_IN_SECONDS
        };

        const int64_t lExpire = (((nExpire < 0) || (nExpire >= timespan_array_len)) ? 0 : array_timespan[nExpire]);
        // --------------------------------------------
/*
    EXPORT  std::string create_market_offer(const std::string  & ASSET_ACCT_ID,
                                            const std::string  & CURRENCY_ACCT_ID,
                                            const int64_t  scale,
                                            const int64_t  minIncrement,
                                            const int64_t  quantity,
                                            const int64_t  price,
                                            const bool      bSelling,
                                            const int64_t  lLifespanInSeconds,  // 0 does default of 86400 == 1 day.
                                            const std::string STOP_SIGN, // If a stop order, must be "<" or ">"
                                            const int64_t ACTIVATION_PRICE); // If a stop order, must be non-zero.
*/

        // --------------------------------------------------------------
        const int64_t     lActualPrice (bIsMarketOrder ? 0         : lPrice);
        const int64_t     lMinIncrement(bIsFillOrKill  ? lQuantity : 1);
        // --------------------------------------------------------------
        const int64_t     lActivationPrice = 0; // For now we're not supporting stop orders in the GUI.
        const std::string str_stop_sign("");    // For now we're not supporting stop orders in the GUI.
        // ----------------------------------------------------------------
        // Do some validation on the account balances.
        //
        if (bIsMarketOrder)
        {
            if (( bIsBid && (lCurrencyAcctBalance <= 0)) || // If I'm buying, but have no currency...
                (!bIsBid && (lAssetAcctBalance    <= 0)))   // ...or if I'm selling, but have no assets...
            {
                QMessageBox::warning(this,
                                     bIsBid ? tr("Currency Account Empty.") : tr("Asset Account Empty."),
                                     bIsBid ? tr("Cannot place bid offer: currency account is empty.") :
                                              tr("Cannot place ask offer: asset account is empty.") );
                return;
            }
        }
        else // Limit order
        {
            const int64_t lHaveBalance = bIsBid ? lCurrencyAcctBalance : lAssetAcctBalance;
            const int64_t lNeedBalance = bIsBid ? (lQuantity * lPrice) : lTotalAsset;

            if (lHaveBalance < lNeedBalance)
            {
                const QString qstrNeededBalance = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(
                                                                             bIsBid ? str_currency_id : str_asset_id,
                                                                             lNeedBalance));
                QString qstrError = bIsBid ?
                            QString("%1 '%2' %3 %4, %5 %6 %7").arg(tr("Cannot place bid offer: currency account")).
                            arg(qstrCurrencyAcctName).arg(tr("contains")).arg(qstrCurrencyAcctBalance).
                            arg(tr("but at least")).arg(qstrNeededBalance).arg(tr("is needed."))
                          :
                            QString("%1 '%2' %3 %4, %5 %6 %7").arg(tr("Cannot place ask offer: asset account")).
                            arg(qstrAssetAcctName).arg(tr("contains")).arg(qstrAssetAcctBalance).
                            arg(tr("but at least")).arg(qstrNeededBalance).arg(tr("is needed."))
                          ;
                QMessageBox::warning(this, bIsBid ? tr("Insufficient Funds.") : tr("Insufficient Assets."), qstrError);
                return;
            }
        }
        // ----------------------------------------------------------------
        // If we're in here, that means the person entered a valid market offer,
        // and then clicked "OK".
        //

        std::string  strResponse;
        {
            MTSpinner theSpinner;
            // --------------------------------------------------------------
            auto action = opentxs::OT::App().API().ServerAction().CreateMarketOffer(opentxs::Identifier(str_asset_acct_id),
            		opentxs::Identifier(str_currency_acct_id),
                    lScale,
					lMinIncrement,
					lTotalAsset,
					lActualPrice,
                    bSelling,
					std::chrono::seconds(lExpire),  // 0 does default of 86400 == 1 day.
                    str_stop_sign, // If a stop order, must be "<" or ">"
                    lActivationPrice); // For stop orders.
            strResponse = action->Run();
        }
        // --------------------------------------------------------
        const std::string strAttempt("create_market_offer");

        int32_t nInterpretReply = opentxs::InterpretTransactionMsgReply(str_notary_id,
                                                                        str_nym_id,
                                                                        str_asset_acct_id,
                                                                        strAttempt, strResponse);
        const bool bPlacedOffer = (1 == nInterpretReply);
        // ---------------------------------------------------------
        if (!bPlacedOffer)
        {
            const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(str_notary_id, str_nym_id);

            // In the cases of -2 and 0, HasUsageCredits already pops up a message box.
            //
            if (((-2) != lUsageCredits) && (0 != lUsageCredits))
                QMessageBox::warning(this,
                                     tr("Failed placing Offer on Market"),
                                     tr("Failed placing offer on market."));
        }
        else
        {
            QMessageBox::information(this,
                                     tr("Success Placing Offer on Market"),
                                     tr("Success placing offer on market. "
                                        "NOTE: Though offers are always added according to turn, "
                                        "it can still take around 30 seconds for an offer to appear on the market. "
                                        "(So wait a little while before clicking 'Refresh.')"));
            // --------------------------------------------------------
//          emit balancesChanged(m_qstrAcctId); // Offers don't appear on market immediately anyway, so no point emitting a signal.
        }
        // -----------------------------------------------------------------
    }
}

// ------------------------------------------------------------------------

void MTOfferDetails::DeleteButtonClicked()
{
    if (m_pOwner && m_pOwner->m_pmapOffers && !m_pOwner->m_qstrCurrentID.isEmpty())
    {
        // Make sure it works without this before I erase it.
        //
//        QString     qstrNotaryID, qstrTransactionID;
//        QStringList theIDs = m_pOwner->m_qstrCurrentID.split(","); // theIDs.at(0) NotaryID, at(1) transaction ID

//        if (2 == theIDs.size()) // Should always be 2...
//        {
//            qstrNotaryID      = theIDs.at(0);
//            qstrTransactionID = theIDs.at(1);
//        }
        // -------------------------------------
        QMap<QString, QVariant>::iterator it_offer = m_pOwner->m_pmapOffers->find(m_pOwner->m_qstrCurrentID);

        if (m_pOwner->m_pmapOffers->end() != it_offer)
        {
            // ------------------------------------------------------
            opentxs::OTDB::OfferDataNym * pOfferData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_offer.value());

            if (NULL != pOfferData) // Should never be NULL.
            {
                const std::string str_notary_id     (pOfferData->notary_id);
                const std::string str_nym_id        (m_pOwner->GetMarketNymID().toStdString());
                const std::string str_asset_acct_id (pOfferData->asset_acct_id);
                // ---------------------------------
                opentxs::String      strTransID(pOfferData->transaction_id);
                const int64_t lTransID  (strTransID.ToLong());
                // ---------------------------------

                std::string  strResponse;
                {
                    MTSpinner theSpinner;
                    // --------------------------------------------------------------
                    auto action = opentxs::OT::App().API().ServerAction().KillMarketOffer(opentxs::Identifier(str_nym_id),
                    		opentxs::Identifier(str_notary_id),
							opentxs::Identifier(str_asset_acct_id),
                            lTransID);
                    strResponse = action->Run();
                }
                // --------------------------------------------------------
                const std::string strAttempt("kill_market_offer");

                int32_t nInterpretReply = opentxs::InterpretTransactionMsgReply(str_notary_id,
                                                                                str_nym_id,
                                                                                str_asset_acct_id,
                                                                                strAttempt, strResponse);
                const bool bKilledOffer = (1 == nInterpretReply);
                // ---------------------------------------------------------
                if (!bKilledOffer)
                {
                    const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(str_notary_id, str_nym_id);

                    // In the cases of -2 and 0, HasUsageCredits already pops up a message box.
                    //
                    if (((-2) != lUsageCredits) && (0 != lUsageCredits))
                        QMessageBox::warning(this,
                                             tr("Failed removing Offer from Market"),
                                             tr("Failed removing offer from market."));
                }
                else
                {
                    QMessageBox::information(this,
                                             tr("Success Removing Offer from Market"),
                                             tr("Success removing offer from market. "));
                    // --------------------------------------------------------
                    m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                    m_pOwner->m_pmapOffers->remove(m_pOwner->m_qstrCurrentID);

                    ClearContents();

                    emit RefreshRecordsAndUpdateMenu();

//                  emit balancesChanged(m_qstrAcctId); // Offers don't appear on market immediately anyway, so no point emitting a signal.
                }
            }
        }
    }
}

// -----------------------------------------------------------------

bool MTOfferDetails::ChooseServer(QString & qstrNotaryID, QString & qstrServerName)
{
    QString qstr_default_id = Moneychanger::It()->get_default_notary_id();
    // -------------------------------------------
    QString qstr_current_id = m_pOwner->GetMarketNotaryID();
    // -------------------------------------------
    const QString qstrAll(tr("all"));

    if (qstr_current_id.isEmpty() || (0 == qstrAll.compare(qstr_current_id)))
        qstr_current_id = qstr_default_id;
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (opentxs::OT::App().API().Exec().GetServerCount() > 0))
        qstr_current_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(0));
    // -------------------------------------------
    // Select from Servers in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = opentxs::OT::App().API().Exec().GetServerCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Server"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            qstrNotaryID   = theChooser.m_qstrCurrentID;
            qstrServerName = theChooser.m_qstrCurrentName;

            return true;
        }
    }
    // --------------
    return false;
}

// ------------------------------------------------------------------------
// For a market offer.
bool MTOfferDetails::getAccountIDs(QString & qstrAssetAcctID, QString & qstrCurrencyAcctID)
{
    qstrAssetAcctID    = ui->lineEditAssetAcctID->text();
    qstrCurrencyAcctID = ui->lineEditCurrencyAcctID->text();

    return (!qstrAssetAcctID.isEmpty() && !qstrCurrencyAcctID.isEmpty());
}



MTOfferDetails::MTOfferDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTOfferDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    ui->lineEditAssetAcct      ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyAcct   ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditAssetAcctID    ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyAcctID ->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    ui->lineEditAcctBalance    ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCurrencyBalance->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
//    ui->tableWidgetTrades ->verticalHeader()->hide();
//    // ----------------------------------
//    ui->tableWidgetTrades->horizontalHeader()->setStretchLastSection(true);
//    ui->tableWidgetTrades->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
//    ui->tableWidgetTrades->setSelectionMode    (QAbstractItemView::SingleSelection);
//    ui->tableWidgetTrades->setSelectionBehavior(QAbstractItemView::SelectRows);

//    ui->tableWidgetTrades->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
//    ui->tableWidgetTrades->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
//    ui->tableWidgetTrades->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
//    ui->tableWidgetTrades->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
//    ui->tableWidgetTrades->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
//    ui->tableWidgetTrades->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);
    // ----------------------------------
    QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

    if (pModel)
    {
        pTradeDataProxy_ = new TradeArchiveProxyModel;

        pTradeDataProxy_->setSourceModel(pModel);

        ui->tableViewTrades->setModel(pTradeDataProxy_);
        ui->tableViewTrades->setSortingEnabled(true);
        ui->tableViewTrades->resizeColumnsToContents();
        ui->tableViewTrades->setEditTriggers(QAbstractItemView::NoEditTriggers);

        ui->tableViewTrades->verticalHeader()->hide();
        ui->tableViewTrades->setAlternatingRowColors(true);

        ui->tableViewTrades->horizontalHeader()->setStretchLastSection(true);
        ui->tableViewTrades->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

        //ui->tableView->setSelectionMode    (QAbstractItemView::SingleSelection);
        ui->tableViewTrades->setSelectionBehavior(QAbstractItemView::SelectRows);
    }
}



void MTOfferDetails::on_toolButtonAssetAcct_clicked()
{
    if (!ui->lineEditAssetAcctID->text().isEmpty())
        emit ShowAccount(ui->lineEditAssetAcctID->text());
}

void MTOfferDetails::on_toolButtonCurrencyAcct_clicked()
{
    if (!ui->lineEditCurrencyAcctID->text().isEmpty())
        emit ShowAccount(ui->lineEditCurrencyAcctID->text());
}

void MTOfferDetails::refresh(QString strID, QString strName)
{
    // -----------------------------------
    ClearTradesGrid();
    // ----------------------------------------
    if (!strID.isEmpty() && (NULL != ui))
    {
        // FYI, contents of strID:
//      QString qstrCompositeID = QString("%1,%2").arg(qstrNotaryID).arg(qstrTransactionID);

        QString     qstrNotaryID, qstrTransactionID;
        QStringList theIDs = strID.split(","); // theIDs.at(0) NotaryID, at(1) transaction ID

        if (2 == theIDs.size()) // Should always be 2...
        {
            qstrNotaryID      = theIDs.at(0);
            qstrTransactionID = theIDs.at(1);
        }
        // -------------------------------------
        if (m_pOwner && m_pOwner->m_pmapOffers)
        {
            QMap<QString, QVariant>::iterator it_offer = m_pOwner->m_pmapOffers->find(strID);

            if (m_pOwner->m_pmapOffers->end() != it_offer)
            {
                // ------------------------------------------------------
                opentxs::OTDB::OfferDataNym * pOfferData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_offer.value());

                if (NULL != pOfferData) // Should never be NULL.
                {
                    bool        bSelling          = pOfferData->selling;
                    // ------------------------------------------------------
                    int64_t     lTotalAssets      = opentxs::OT::App().API().Exec().StringToLong(pOfferData->total_assets);
                    int64_t     lFinished         = opentxs::OT::App().API().Exec().StringToLong(pOfferData->finished_so_far);
                    // ------------------------------------------------------
                    int64_t     lStillAvailable   = lTotalAssets - lFinished;
                    // ------------------------------------------------------
                    int64_t     lMinimumIncrement = opentxs::OT::App().API().Exec().StringToLong(pOfferData->minimum_increment);
                    // ------------------------------------------------------
                    int64_t     lScale            = opentxs::OT::App().API().Exec().StringToLong(pOfferData->scale);
                    std::string str_scale         = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lScale);
                    // ------------------------------------------------------
                    int64_t     lPrice            = opentxs::OT::App().API().Exec().StringToLong(pOfferData->price_per_scale);
                    std::string str_price         = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->currency_type_id, lPrice);
                    // ------------------------------------------------------
                    QString qstrPrice(tr("market order"));

                    if (lPrice > 0)
                        qstrPrice = QString("%1: %2").arg(tr("Price")).arg(QString::fromStdString(str_price));
                    // ------------------------------------------------------
                    QString qstrFormattedScale    = QString::fromStdString(str_scale);

                    if (lScale > 1)
                        qstrPrice += QString(" (%1 %2)").arg(tr("per")).arg(qstrFormattedScale);
                    // ------------------------------------------------------
                    QString qstrMinimumIncrement  = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lMinimumIncrement));
                    QString qstrTotalAssets       = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lTotalAssets));
                    QString qstrSoldOrPurchased   = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lFinished));
                    QString qstrStillAvailable    = QString::fromStdString(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lStillAvailable));
                    // ------------------------------------------------------
                    std::string str_asset_name    = opentxs::OT::App().API().Exec().GetAssetType_Name(pOfferData->instrument_definition_id);
                    // -----------------------------------------------------------------------
                    time_t tValidFrom      = static_cast<time_t>(opentxs::OT::App().API().Exec().StringToLong(pOfferData->valid_from));
                    time_t tValidTo        = static_cast<time_t>(opentxs::OT::App().API().Exec().StringToLong(pOfferData->valid_to));
                    // -----------------------------------------------------------------------
                    QDateTime qdate_from   = QDateTime::fromTime_t(tValidFrom);
                    QDateTime qdate_to     = QDateTime::fromTime_t(tValidTo);
                    // -----------------------------------------------------------------------
                    QString   qstrDateFrom = qdate_from.toString(QString("MMM d yyyy hh:mm:ss"));
                    QString   qstrDateTo   = qdate_to  .toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    ui->labelValidFrom ->setText(qstrDateFrom);
                    ui->labelValidTo   ->setText(qstrDateTo);
                    // ------------------------------------------------------
                    ui->labelAvailable->setText(qstrStillAvailable);
                    ui->labelMinInc   ->setText(qstrMinimumIncrement);
                    // ------------------------------------------------------
                    QString qstrBuySell = bSelling ? tr("Sell") : tr("Buy");
                    QString qstrAmounts;

                    qstrAmounts = QString("%1").
                            arg(qstrTotalAssets);
                    // ------------------------------------------------------
                    QString qstrCompleted("");

                    if (lFinished > 0)
                        qstrCompleted = QString("%1: %2").
                                arg(tr("completed")).
                                arg(qstrSoldOrPurchased);
                    // --------------------------
        //          "Buy Silver Grams: 300g (40g finished so far)";
                    //
                    QString qstrOfferName = QString("%1 %2: %3").
                            arg(qstrBuySell).
                            arg(QString::fromStdString(str_asset_name)).
                            arg(qstrAmounts);
                    // -------------------------------------
                    QString qstrTrans = QString("%1# %2").arg(tr("Trans")).arg(qstrTransactionID);
//                    QString qstrTrans = QString("<font size=1 color=grey>%1#</font> %2").arg(tr("Trans")).arg(qstrTransactionID);
                    // -------------------------------------
                    QWidget * pHeaderWidget = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrPrice, qstrOfferName,
                                                                                      qstrCompleted, qstrTrans, ":/icons/icons/assets.png", false);

                    pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

                    if (m_pHeaderWidget)
                    {
                        ui->verticalLayout->removeWidget(m_pHeaderWidget);

                        m_pHeaderWidget->setParent(NULL);
                        m_pHeaderWidget->disconnect();
                        m_pHeaderWidget->deleteLater();

                        m_pHeaderWidget = NULL;
                    }
                    ui->verticalLayout->insertWidget(0, pHeaderWidget);
                    m_pHeaderWidget = pHeaderWidget;
                    // ------------------------------------------------------
                    // For these, we're doing it this way because we're calculating
                    // these values across multiple markets. (The same market type,
                    // but across multiple servers.)
                    //
                    // ------------------------------------------------------
                    ui->lineEditAssetAcct      ->setText(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(pOfferData->asset_acct_id)));
                    ui->lineEditCurrencyAcct   ->setText(QString::fromStdString(opentxs::OT::App().API().Exec().GetAccountWallet_Name(pOfferData->currency_acct_id)));
                    // ------------------------------------------------------
                    ui->lineEditAssetAcctID    ->setText(QString::fromStdString(pOfferData->asset_acct_id));
                    ui->lineEditCurrencyAcctID ->setText(QString::fromStdString(pOfferData->currency_acct_id));
                    // ------------------------------------------------------
                    const int64_t lAcctBalance     = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(pOfferData->asset_acct_id);
                    const int64_t lCurrencyBalance = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(pOfferData->currency_acct_id);

                    const std::string str_acct_balance     = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id,    lAcctBalance);
                    const std::string str_currency_balance = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->currency_type_id, lCurrencyBalance);

                    ui->lineEditAcctBalance    ->setText(QString::fromStdString(str_acct_balance));
                    ui->lineEditCurrencyBalance->setText(QString::fromStdString(str_currency_balance));
                     // ------------------------------------------------------
                    PopulateNymTradesGrid (strID, m_pOwner->GetMarketNymID(), *(m_pOwner->m_pmapOffers));
                    // ------------------------------------------------------
                }
            }
        }
        // ----------------------------------
        FavorLeftSideForIDs();
        // ----------------------------------
    }
    else
        ClearContents();
}


// ----------------------------------------------------

//// Caller must delete.
//opentxs::OTDB::TradeListNym * MTOfferDetails::LoadTradeListForNym(QString qstrNotaryID, QString qstrNymID)
//{
//    opentxs::OTDB::TradeListNym * pTradeList = NULL;
//    opentxs::OTDB::Storable     * pStorable  = NULL;
//    // ------------------------------------------
//    QString qstrMarketID = m_pOwner->GetMarketID();
//    // ------------------------------------------
//    if (!m_pOwner || qstrNotaryID.isEmpty() || qstrNymID.isEmpty() || qstrMarketID.isEmpty())
//        return NULL;
//    // ------------------------------------------
//    if (opentxs::OTDB::Exists("nyms", "trades", qstrNotaryID.toStdString(), qstrNymID.toStdString()))
//    {
//        pStorable = opentxs::OTDB::QueryObject(opentxs::OTDB::STORED_OBJ_TRADE_LIST_NYM, "nyms", "trades",
//                                      qstrNotaryID.toStdString(), qstrNymID.toStdString());
//        if (nullptr == pStorable) {
//            return NULL;
//        }
//        // -------------------------------
//        pTradeList = opentxs::OTDB::TradeListNym::ot_dynamic_cast(pStorable);

//        if (NULL == pTradeList) {
//            delete pStorable;
//        }
//    }

//    return pTradeList;
//}


void MTOfferDetails::ClearTradesGrid()
{
    pTradeDataProxy_->setFilterNymId(QString(""));
    pTradeDataProxy_->setFilterOfferId(0);
}

void MTOfferDetails::PopulateNymTradesGrid(QString & qstrID, QString qstrNymID, QMap<QString, QVariant> & OFFER_MAP)
{
    QMap<QString, QVariant>::iterator it_offer = OFFER_MAP.find(qstrID);
    // -----------------------------
    if (OFFER_MAP.end() != it_offer)
    {
        opentxs::OTDB::OfferDataNym * pOfferData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_offer.value());

        if (NULL != pOfferData) // Should never be NULL.
        {
            QPointer<ModelTradeArchive> pModel = DBHandler::getInstance()->getTradeArchiveModel();

            if (pModel && pTradeDataProxy_)
            {
                pModel->updateDBFromOT();

                int64_t lOfferID = opentxs::OT::App().API().Exec().StringToLong(pOfferData->transaction_id);

                pTradeDataProxy_->setFilterNymId(qstrNymID);
                pTradeDataProxy_->setFilterOfferId(lOfferID);
                pTradeDataProxy_->setFilterIsBid(!pOfferData->selling);
            }
        }
    }
}


/*
void MTOfferDetails::PopulateNymTradesWidget(QString & qstrID, QString qstrNymID, QMap<QString, QVariant> & OFFER_MAP)
{
    // ------------------------------------------------------------------------
//    this->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetTrades->blockSignals(true);
    // -----------------------------------
    int nTradesRowCount  = 0;
    // -----------------------------------
    int nTradesGridIndex = 0;
    // -----------------------------------
    QMap<QString, QVariant>::iterator it_offer = OFFER_MAP.find(qstrID);
    // -----------------------------
    if (OFFER_MAP.end() != it_offer)
    {
        opentxs::OTDB::OfferDataNym * pOfferData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_offer.value());

        if (NULL != pOfferData) // Should never be NULL.
        {
            std::string & str_server         = pOfferData->notary_id;
            std::string   str_server_display = opentxs::OT::App().API().Exec().GetServer_Name(str_server);
            QString       qstrServerName     = QString::fromStdString(str_server_display);
            // -----------------------------------------
            int64_t lScale = opentxs::OT::App().API().Exec().StringToLong(pOfferData->scale);
            // -----------------------------------------
            QTableWidgetItem * pPriceHeader = ui->tableWidgetTrades->horizontalHeaderItem(0);

            if (NULL != pPriceHeader)
            {
                const std::string str_price_per_scale(opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id,
                                                                               lScale));
                pPriceHeader->setText(QString("%1 %2").arg(tr("Actual Price per")).
                                      arg(QString::fromStdString(str_price_per_scale)));
            }
            // -----------------------------------------
            QTableWidgetItem * pAmountHeader = ui->tableWidgetTrades->horizontalHeaderItem(1);

            if (NULL != pAmountHeader)
            {
                if (pOfferData->selling)
                    pAmountHeader->setText(tr("Sold"));
                else
                    pAmountHeader->setText(tr("Purchased"));
            }
            // -----------------------------------------
            QTableWidgetItem * pCurrencyHeader = ui->tableWidgetTrades->horizontalHeaderItem(2);

            if (NULL != pCurrencyHeader)
            {
                if (pOfferData->selling)
                    pCurrencyHeader->setText(tr("Received"));
                else
                    pCurrencyHeader->setText(tr("Paid"));
            }
            // -----------------------------------------
            opentxs::OTDB::TradeListNym * pTradeList = LoadTradeListForNym(
                                                                  QString::fromStdString(str_server),
                                                                  qstrNymID);
            std::unique_ptr<opentxs::OTDB::TradeListNym> theAngel(pTradeList);

            if (NULL != pTradeList)
            {
                size_t nTradeDataCount = pTradeList->GetTradeDataNymCount();
                // -------------------------------------
                nTradesRowCount += static_cast<int>(nTradeDataCount);
                // -------------------------------------
                ui->tableWidgetTrades->setRowCount(nTradesRowCount);
                // -------------------------------------
                for (size_t trade_index = 0; trade_index < nTradeDataCount; ++trade_index)
                {
                    opentxs::OTDB::TradeDataNym * pTradeData = pTradeList->GetTradeDataNym(trade_index);

                    if (NULL == pTradeData) // Should never happen.
                    {
                        ui->tableWidgetTrades->setRowCount(ui->tableWidgetTrades->rowCount() - 1);
                        continue;
                    }
                    // -----------------------------------------------------------------------
                    int64_t lOfferID = opentxs::OT::App().API().Exec().StringToLong(pOfferData->transaction_id);
                    int64_t lTradeID = opentxs::OT::App().API().Exec().StringToLong(pTradeData->transaction_id);

                    if (lOfferID != lTradeID)
                    {
//                        qDebug() << QString("Showing trades for Offer %1; skipping trade receipt with trans number %2.")
//                                    .arg(lOfferID).arg(lTradeID);

                        ui->tableWidgetTrades->setRowCount(ui->tableWidgetTrades->rowCount() - 1);
                        continue;
                    }
                    // -----------------------------------------------------------------------
                    QString qstrUpdatedID     = QString::fromStdString(pTradeData->updated_id);
                    // -----------------------------------------------------------------------
//                  time_t tDate = static_cast<time_t>(opentxs::OT::App().API().Exec().StringToLong(pTradeData->date));
                    time64_t tDate = static_cast<time64_t>(opentxs::OT::App().API().Exec().StringToLong(pTradeData->date));

                    QDateTime qdate_added   = QDateTime::fromTime_t(tDate);
                    QString   qstrDateAdded = qdate_added.toString(QString("MMM d yyyy hh:mm:ss"));
                    // -----------------------------------------------------------------------
                    std::string & str_price         = pTradeData->price;

                    int64_t       lPrice            = opentxs::OT::App().API().Exec().StringToLong(str_price); // this price is "per scale"

                    if (lPrice < 0)
                        lPrice *= (-1);

                    std::string   str_price_display = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->currency_type_id, lPrice);

                    QString qstrPrice = QString::fromStdString(str_price_display);
                    // -----------------------------------------------------------------------
                    std::string & str_amount_sold    = pTradeData->amount_sold;
                    int64_t       lQuantity          = opentxs::OT::App().API().Exec().StringToLong(str_amount_sold); // Total amount of asset sold.

                    if (lQuantity < 0)
                        lQuantity *= (-1);

                    std::string   str_amount_display = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->instrument_definition_id, lQuantity);

                    QString qstrAmountSold = QString::fromStdString(str_amount_display);
                    // -----------------------------------------------------------------------
                    std::string & str_currency_paid   = pTradeData->currency_paid;
                    int64_t       lPayQuantity        = opentxs::OT::App().API().Exec().StringToLong(str_currency_paid); // Total currency paid

                    if (lPayQuantity < 0)
                        lPayQuantity *= (-1);

                    std::string   str_paid_display    = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->currency_type_id, lPayQuantity);

                    QString qstrCurrencyPaid = QString::fromStdString(str_paid_display);
                    // -----------------------------------------------------------------------
                    // If the "actual price" is zero, we'll interpolate it.
                    //
                    if (0 == lPrice)
                    {
                        if ((lQuantity != 0) && (lPayQuantity != 0) && (lScale != 0))
                        {
                            lPrice = lPayQuantity / (lQuantity / lScale);

                            str_price_display = opentxs::OT::App().API().Exec().FormatAmount(pOfferData->currency_type_id, lPrice);
                            qstrPrice = QString::fromStdString(str_price_display);
                        }
                    }
                    // -----------------------------------------------------------------------
                    QLabel * pLabelPrice        = new QLabel(qstrPrice);
                    QLabel * pLabelAmountSold   = new QLabel(qstrAmountSold);
                    QLabel * pLabelCurrencyPaid = new QLabel(qstrCurrencyPaid);
                    QLabel * pLabelDateAdded    = new QLabel(QString("<small>%1</small>").arg(qstrDateAdded));
                    QLabel * pLabelTransID      = new QLabel(qstrUpdatedID);
                    QLabel * pLabelServer       = new QLabel(qstrServerName);
                    // -----------------------------------------------------------------------
                    pLabelPrice       ->setAlignment(Qt::AlignCenter);
                    pLabelAmountSold  ->setAlignment(Qt::AlignCenter);
                    pLabelCurrencyPaid->setAlignment(Qt::AlignCenter);
                    pLabelDateAdded   ->setAlignment(Qt::AlignCenter);
                    pLabelTransID     ->setAlignment(Qt::AlignCenter);
                    pLabelServer      ->setAlignment(Qt::AlignCenter);
                    // -----------------------------------------------------------------------
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 0, pLabelPrice     );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 1, pLabelAmountSold);
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 2, pLabelCurrencyPaid);
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 3, pLabelDateAdded );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 4, pLabelTransID   );
                    ui->tableWidgetTrades->setCellWidget ( nTradesGridIndex, 5, pLabelServer    );
                    // -----------------------------------------------------------------------
                    ++nTradesGridIndex;
                    // -----------------------------------------------------------------------
                } // for (trades)
                // -----------------------------------------------------------------------
            } // if (NULL != pTradeList)
        } // if (NULL != pOfferData)
    }
    // -----------------------------------------------------
//    this->blockSignals(false);
    // -----------------------------------
    ui->tableWidgetTrades->blockSignals(false);
    // -----------------------------------------------------
    if (ui->tableWidgetTrades->rowCount() > 0)
        ui->tableWidgetTrades->setCurrentCell(0, 0);
    // -----------------------------------------------------
}
*/

// ------------------------------------------------------------------------

void MTOfferDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        // -------------------------------------
        ui->lineEditAssetAcct      ->home(false);
        ui->lineEditCurrencyAcct   ->home(false);
        // -------------------------------------
        ui->lineEditAssetAcctID    ->home(false);
        ui->lineEditCurrencyAcctID ->home(false);
        // -------------------------------------
        ui->lineEditAcctBalance    ->home(false);
        ui->lineEditCurrencyBalance->home(false);
    }
}

void MTOfferDetails::ClearContents()
{
    // -------------------------------------
    ui->lineEditAssetAcct     ->setText("");
    ui->lineEditCurrencyAcct  ->setText("");
    // -------------------------------------
    ui->lineEditAssetAcctID   ->setText("");
    ui->lineEditCurrencyAcctID->setText("");
    // -------------------------------------
    ui->lineEditAcctBalance    ->setText("");
    ui->lineEditCurrencyBalance->setText("");
    // -------------------------------------
    ui->labelAvailable->setText("");
    ui->labelMinInc   ->setText("");
    ui->labelValidFrom->setText("");
    ui->labelValidTo  ->setText("");
    // -------------------------------------
    ClearTradesGrid();
}

// ------------------------------------------------------

bool MTOfferDetails::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
//    else
//    {
        // standard event processing
//        return QWidget::eventFilter(obj, event);
        return MTEditDetails::eventFilter(obj, event);

        // NOTE: Since the base class has definitely already installed this
        // function as the event filter, I must assume that this version
        // is overriding the version in the base class.
        //
        // Therefore I call the base class version here, since as it's overridden,
        // I don't expect it will otherwise ever get called.
//    }
}

// ------------------------------------------------------

MTOfferDetails::~MTOfferDetails()
{
    delete ui;
}
