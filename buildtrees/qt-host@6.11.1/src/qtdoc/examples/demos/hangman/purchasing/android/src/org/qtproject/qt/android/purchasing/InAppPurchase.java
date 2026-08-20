// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

package org.qtproject.qt.android.purchasing;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;
import com.android.billingclient.api.QueryProductDetailsParams.Product;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingFlowParams.ProductDetailsParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.Purchase.PurchaseState;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.ProductDetails;

/***********************************************************************
 ** More info: https://developer.android.com/google/play/billing
 ** Add Dependencies below to build.gradle file:

dependencies {
    def billingVersion = "6.0.1"
    implementation "com.android.billingclient:billing:$billingVersion"
}

***********************************************************************/

public class InAppPurchase implements PurchasesUpdatedListener
{
    private Context m_context;
    private long m_nativePointer;
    private String m_publicKey = null;
    private int purchaseRequestCode;

    private BillingClient billingClient;

    public static final int RESULT_OK = BillingClient.BillingResponseCode.OK;
    public static final int RESULT_USER_CANCELED = BillingClient.BillingResponseCode.USER_CANCELED;
    public static final String TYPE_INAPP = BillingClient.ProductType.INAPP;
    public static final String TAG = "InAppPurchase";

    // Should be in sync with InAppTransaction::FailureReason
    public static final int FAILUREREASON_NOFAILURE    = 0;
    public static final int FAILUREREASON_USERCANCELED = 1;
    public static final int FAILUREREASON_ERROR        = 2;

    public InAppPurchase(Context context, long nativePointer)
    {
        m_context = context;
        m_nativePointer = nativePointer;
    }

    public void initializeConnection(){
        billingClient = BillingClient.newBuilder(m_context)
                .enablePendingPurchases()
                .setListener(this)
                .build();
        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() == RESULT_OK)
                    purchasedProductsQueried(m_nativePointer);
                else
                    Log.w(TAG, "Can not connect to Billing service");
            }
            @Override
            public void onBillingServiceDisconnected() {
                Log.w(TAG, "Billing service disconnected");
            }
        });
    }

    @Override
    public void onPurchasesUpdated(BillingResult billingResult, List<Purchase> purchases) {
        int responseCode = billingResult.getResponseCode();
        if (purchases == null || purchases.isEmpty()) {
            purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, "Data missing from result");
            return;
        }
        if (responseCode == RESULT_OK) {
            handlePurchase(purchases);
        } else if (responseCode == RESULT_USER_CANCELED) {
            purchaseFailed(
                    purchaseRequestCode, FAILUREREASON_USERCANCELED, "User canceled the purchase.");
        } else {
            String errorString = getErrorString(responseCode);
            purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, errorString);
        }
    }

    //Get list of purchases from onPurchasesUpdated
    private void handlePurchase(List<Purchase> purchases) {
        for (Purchase purchase : purchases) {
            try {
                if (m_publicKey != null && !Security.verifyPurchase(
                        m_publicKey, purchase.getOriginalJson(), purchase.getSignature())) {
                    purchaseFailed(
                            purchaseRequestCode,
                            FAILUREREASON_ERROR,
                            "Signature could not be verified");
                    return;
                }
                int purchaseState = purchase.getPurchaseState();
                if (purchaseState != PurchaseState.PURCHASED) {
                    purchaseFailed(
                            purchaseRequestCode,
                            FAILUREREASON_ERROR,
                            "Unexpected purchase state in result");
                    return;
                }
            } catch (Exception e) {
                e.printStackTrace();
                purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, e.getMessage());
            }
            purchaseSucceeded(
                purchaseRequestCode,
                purchase.getSignature(),
                purchase.getOriginalJson(),
                purchase.getPurchaseToken(),
                purchase.getOrderId(),
                purchase.getPurchaseTime());
        }
    }

    private QueryProductDetailsParams productDetailsResponse(String[] strIdList) {
        List<Product> productIdList = new ArrayList<>();
        for (String productId : strIdList) {
            productIdList.add(
                Product.newBuilder()
                .setProductId(productId)
                .setProductType(TYPE_INAPP).build());
        }
        QueryProductDetailsParams queryProductDetailsParams = QueryProductDetailsParams.newBuilder()
                .setProductList(productIdList)
                .build();
        return queryProductDetailsParams;
    }

    public void queryDetails(final String[] productIds) {
        QueryProductDetailsParams queryProductDetailsParams = productDetailsResponse(productIds);
        billingClient.queryProductDetailsAsync(
                queryProductDetailsParams, new ProductDetailsResponseListener() {
            @Override
            public void onProductDetailsResponse(
                    BillingResult billingResult,
                    List<ProductDetails> productDetailsResponseList) {
                int responseCode = billingResult.getResponseCode();
                List<String> productIdList = new ArrayList<String>();
                if (responseCode != RESULT_OK) {
                    Log.e(TAG, "queryDetails: Couldn't retrieve product details.");
                }
                if (productDetailsResponseList == null || productDetailsResponseList.isEmpty()) {
                    Log.e(TAG, "queryDetails: No details list in the response.");
                }
                for (ProductDetails productDetails : productDetailsResponseList) {
                    try {
                        String queriedProductId = productDetails.getProductId();
                        String queriedPrice = productDetails
                                .getOneTimePurchaseOfferDetails()
                                .getFormattedPrice();
                        String queriedTitle = productDetails.getTitle();
                        String queriedDescription = productDetails.getDescription();
                        if (queriedProductId.equals("") ||
                                queriedPrice.equals("") ||
                                queriedTitle.equals("") ||
                                queriedDescription.equals("")) {
                            Log.e(TAG, "Data missing from product details.");
                            queryFailed(m_nativePointer, queriedProductId);
                        } else {
                            productIdList.add(queriedProductId);
                            registerProduct(m_nativePointer,
                                    queriedProductId,
                                    queriedPrice,
                                    queriedTitle,
                                    queriedDescription);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "queryDetails: ", e);
                    }
                }
                queryPurchasedProducts(productIdList);
            }
        });
    }

    public void launchBillingFlow(String identifier, int purchaseRequestCode){
        QueryProductDetailsParams queryProductDetailsParams =
                productDetailsResponse(new String[]{identifier});
        billingClient.queryProductDetailsAsync(
                queryProductDetailsParams, new ProductDetailsResponseListener() {
            public void onProductDetailsResponse(
                    BillingResult billingResult,
                    List<ProductDetails> productDetailsResponseList) {
                int responseCode = billingResult.getResponseCode();
                if (responseCode != RESULT_OK) {
                    String errorString = getErrorString(purchaseRequestCode);
                    purchaseFailed(purchaseRequestCode, FAILUREREASON_ERROR, errorString);
                    return;
                } else if (productDetailsResponseList == null
                        || productDetailsResponseList.isEmpty()) {
                    purchaseFailed(
                            purchaseRequestCode,
                            FAILUREREASON_ERROR,
                            "Data missing from result");
                    return;
                }
                List<ProductDetailsParams> productDetailsParamsList = new ArrayList<>();
                productDetailsParamsList.add(
                        ProductDetailsParams.newBuilder()
                                .setProductDetails(productDetailsResponseList.get(0))
                                .build()
                );
                BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                        .setProductDetailsParamsList(productDetailsParamsList)
                        .build();

                billingClient.launchBillingFlow((Activity) m_context, billingFlowParams);
            }
        });
    }

    public void consumePurchase(String purchaseToken){
        ConsumeResponseListener listener = new ConsumeResponseListener() {
            @Override
            public void onConsumeResponse(BillingResult billingResult, String purchaseToken) {
                if (billingResult.getResponseCode() != RESULT_OK) {
                    Log.e(TAG,
                            "Unable to consume purchase. Response code: "
                            + billingResult.getResponseCode());
                }
            }
        };
        ConsumeParams consumeParams = ConsumeParams
                .newBuilder()
                .setPurchaseToken(purchaseToken)
                .build();
        billingClient.consumeAsync(consumeParams, listener);
    }

    public void acknowledgeUnlockablePurchase(String purchaseToken){
        AcknowledgePurchaseParams acknowledgePurchaseParams = AcknowledgePurchaseParams
                .newBuilder()
                .setPurchaseToken(purchaseToken)
                .build();
        AcknowledgePurchaseResponseListener acknowledgePurchaseResponseListener =
                new AcknowledgePurchaseResponseListener() {
            @Override
            public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                if (billingResult.getResponseCode() != RESULT_OK){
                    Log.e(TAG,
                            "Unable to acknowledge purchase. Response code: "
                            + billingResult.getResponseCode());
                }
            }
        };
        billingClient.acknowledgePurchase(
                acknowledgePurchaseParams,
                acknowledgePurchaseResponseListener);
    }

    public void queryPurchasedProducts(List<String> productIdList) {
        billingClient.queryPurchasesAsync(
            QueryPurchasesParams.newBuilder().setProductType(TYPE_INAPP).build(),
                    new PurchasesResponseListener() {
                public void onQueryPurchasesResponse(
                        BillingResult billingResult,
                        List<Purchase> purchases) {
                    if (billingResult.getResponseCode() != RESULT_OK) {
                        Log.e(TAG, "queryPurchasedProducts: Couldn't retrieve purchased item.");
                        return;
                    }
                    if (purchases == null || purchases.isEmpty()) {
                        Log.e(TAG, "queryPurchasedProducts: No purchase list in response.");
                        return;
                    }
                    for (Purchase purchase : purchases) {
                        if (productIdList.contains(purchase.getProducts().get(0))) {
                            registerPurchased(m_nativePointer,
                                purchase.getProducts().get(0),
                                purchase.getSignature(),
                                purchase.getOriginalJson(),
                                purchase.getPurchaseToken(),
                                purchase.getDeveloperPayload(),
                                purchase.getPurchaseTime());
                        }
                    }
                }
            }
        );
    }

    private String getErrorString(int responseCode){
        String errorString;
        switch (responseCode) {
            case BillingClient.BillingResponseCode.BILLING_UNAVAILABLE:
                    errorString = "Billing unavailable"; break;
            case BillingClient.BillingResponseCode.ITEM_UNAVAILABLE:
                    errorString = "Item unavailable"; break;
            case BillingClient.BillingResponseCode.DEVELOPER_ERROR:
                    errorString = "Developer error"; break;
            case BillingClient.BillingResponseCode.ERROR:
                    errorString = "Fatal error occurred"; break;
            case BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED:
                    errorString = "Item already owned"; break;
            case BillingClient.BillingResponseCode.ITEM_NOT_OWNED:
                    errorString = "Item not owned"; break;
            default: errorString = "Unknown billing error " + responseCode; break;
        };
        return errorString;
    }

    public void setPublicKey(String publicKey)
    {
        m_publicKey = publicKey;
    }

    private void purchaseFailed(int requestCode, int failureReason, String errorString)
    {
        purchaseFailed(m_nativePointer, requestCode, failureReason, errorString);
    }


    private void purchaseSucceeded(int requestCode,
                                   String signature,
                                   String purchaseData,
                                   String purchaseToken,
                                   String orderId,
                                   long timestamp)
    {
        purchaseSucceeded(
                m_nativePointer,
                requestCode,
                signature,
                purchaseData,
                purchaseToken,
                orderId,
                timestamp);
    }

    private native static void queryFailed(long nativePointer, String productId);
    private native static void purchasedProductsQueried(long nativePointer);
    private native static void registerProduct(long nativePointer,
                                               String productId,
                                               String price,
                                               String title,
                                               String description);
    private native static void purchaseFailed(long nativePointer,
                                              int requestCode,
                                              int failureReason,
                                              String errorString);
    private native static void purchaseSucceeded(long nativePointer,
                                                 int requestCode,
                                                 String signature,
                                                 String data,
                                                 String purchaseToken,
                                                 String orderId,
                                                 long timestamp);
    private native static void registerPurchased(long nativePointer,
                                                 String identifier,
                                                 String signature,
                                                 String data,
                                                 String purchaseToken,
                                                 String orderId,
                                                 long timestamp);
}
