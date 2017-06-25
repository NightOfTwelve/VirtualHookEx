package io.virtualhook;

import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import android.os.Environment;
import android.util.Log;

public class LogUtil {
	public static final String LOG_ROOT = Environment
			.getExternalStorageDirectory().getPath() + "/Download/";
	private static final boolean allowSaveLog = false;
	public static boolean allowD = true;
	public static boolean allowE = true;
	public static boolean allowI = true;
	public static boolean allowV = true;
	public static boolean allowW = true;
	public static boolean allowWtf = true;

	public static void v(final String content) {
		if (allowV) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.v(tag, content);
			saveToFile(tag, content);
		}
	}

	public static void v(final String content, final Throwable tr) {
		if (allowV) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.v(tag, content, tr);
			saveToFile(tag, content, tr);
		}
	}

	public static void i(final String content) {
		if (allowI) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.i(tag, content);
			saveToFile(tag, content);
		}
	}

	public static void i(final String content, final Throwable tr) {
		if (allowI) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.i(tag, content, tr);
			saveToFile(tag, content, tr);
		}
	}

	public static void d(final String content) {
		if (allowD) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.d(tag, content);
			saveToFile(tag, content);
		}
	}

	public static void d(final String content, final Throwable tr) {
		if (allowD) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.d(tag, content, tr);
			saveToFile(tag, content, tr);
		}
	}

	public static void e(final String content) {
		if (allowE) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.e(tag, content);
			saveToFile(tag, content);
		}
	}

	public static void e(final String content, final Throwable tr) {
		if (allowE) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.e(tag, content, tr);
			saveToFile(tag, content, tr);
		}
	}

	public static void w(final String content) {
		if (allowW) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.w(tag, content);
			saveToFile(tag, content);
		}
	}

	public static void w(final Throwable tr) {
		if (allowW) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.w(tag, tr);
			saveToFile(tag, tr);
		}
	}

	public static void w(final String content, final Throwable tr) {
		if (allowW) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.w(tag, content, tr);
			saveToFile(tag, content, tr);
		}
	}

	public static void wtf(final String content) {
		if (allowWtf) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.wtf(tag, content);
			saveToFile(tag, content);
		}
	}

	public static void wtf(final Throwable tr) {
		if (allowWtf) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.wtf(tag, tr);
			saveToFile(tag, tr);
		}
	}

	public static void wtf(final String content, final Throwable tr) {
		if (allowWtf) {
			final String tag = generateTag(getCallerStackTraceElement());
			Log.wtf(tag, content, tr);
			saveToFile(tag, content, tr);
		}
	}

	private static StackTraceElement getCallerStackTraceElement() {
		return Thread.currentThread().getStackTrace()[4];
	}

	private static String generateTag(final StackTraceElement caller) {
		final String callerClazzName = caller.getClassName();
		return String
				.format(Locale.SIMPLIFIED_CHINESE, "%s.%s(Line:%d)",
						callerClazzName.substring(callerClazzName
								.lastIndexOf(".") + 1), caller.getMethodName(),
						caller.getLineNumber());
	}

	private static String formatException(final Throwable t) {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		t.printStackTrace(new PrintStream(out));

		final String exp = out.toString();
		try {
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return exp;
	}

	public static void saveToFile(final String tag, final Throwable tr) {
		if (allowSaveLog) {
			synchronized (LogUtil.class) {
				saveToFileSynchronized(tag, formatException(tr));
			}
		}
	}

	public static void saveToFile(final String tag, final String msg,
			final Throwable tr) {
		if (allowSaveLog) {
			synchronized (LogUtil.class) {
				saveToFileSynchronized(tag, msg + "\n" + formatException(tr));
			}
		}
	}

	public static void saveToFile(final String tag, final String msg) {
		if (allowSaveLog) {
			synchronized (LogUtil.class) {
				saveToFileSynchronized(tag, msg);
			}
		}
	}

	private static void saveToFileSynchronized(final String tag,
			final String msg) {
		System.setProperty("user.timezone", "Asia/Shanghai");
		TimeZone.setDefault(TimeZone.getTimeZone("Asia/Shanghai")); // GMT + 8

		final Date date = new Date();
		final SimpleDateFormat dateFormat = new SimpleDateFormat("",
				Locale.SIMPLIFIED_CHINESE);
		dateFormat.applyPattern("yyyy-MM-dd");
		final String filename = dateFormat.format(date) + ".log";
		dateFormat.applyPattern("[HH:mm:ss "
				+ String.format(Locale.SIMPLIFIED_CHINESE, "% 5d/% 5d",
						android.os.Process.myPid(), android.os.Process.myTid())
				+ "]");
		final String time = dateFormat.format(date);
		final File file = ensurePathCreated(LOG_ROOT + filename);
		BufferedWriter out = null;
		try {
			out = new BufferedWriter(new OutputStreamWriter(
					new FileOutputStream(file, true)));
			out.write(time + " " + tag + " " + msg + "\n");
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (out != null) {
				try {
					out.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	private static File ensurePathCreated(final String file) {
		final File f = new File(file);
		if (!f.exists()) {
			f.getParentFile().mkdirs();
			try {
				f.createNewFile();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return f;
	}
}